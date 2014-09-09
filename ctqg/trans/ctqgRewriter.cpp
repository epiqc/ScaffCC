//-------------------------------------------------------------------------
//
// ctqgRewriter.cpp: Source-to-source transformation sample with Clang,
// using Rewriter - the code rewriting interface.
//
// Jeff Heckey (jheckey@ece.ucsb.edu)
//
// Borrowed and modified from:
// Eli Bendersky (eliben@gmail.com)
//
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
//#include <regex>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Rewriter.h"
#include "clang/Rewrite/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace llvm;
using namespace std;


// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
public:
    MyASTVisitor(Rewriter &R)
        : TheRewriter(R)
    {}

    bool VisitFunctionDecl(FunctionDecl *f) {
		// Only function definitions (with bodies), not declarations.
		if (f->hasBody()) {
			Stmt *FuncBody = f->getBody();

			// Function name
			DeclarationName DeclName = f->getNameInfo().getName();
			string FuncName = DeclName.getAsString();

            // Type name as string
            QualType QT = f->getResultType();
            string TypeStr = QT.getAsString();

            if (TypeStr.compare("cat") != 0) {
                // Add comment before
                stringstream SSBefore;
                SSBefore << "// Begin function " << FuncName << " returning "
                         << TypeStr << "\n";
                SourceLocation ST = f->getSourceRange().getBegin();
                TheRewriter.InsertText(ST, SSBefore.str(), true, true);

                // And after
                stringstream SSAfter;
                SSAfter << "\n// End function " << FuncName << "\n";
                ST = FuncBody->getLocEnd().getLocWithOffset(1);
                TheRewriter.InsertText(ST, SSAfter.str(), true, true);
            } else {
                SourceRange FR = f->getSourceRange();
                SourceLocation PL;
                stringstream SSRepl;
                stringstream SSDefn;
                ParmVarDecl* param;

				// Move declaration to buffer
				ctqgBuffer << TheRewriter.ConvertToString(f->getBody());

                // Remove definition
                FR = f->getSourceRange();
                param = f->getParamDecl(f->getNumParams()-1);
                PL = FuncBody->getLocStart();
                SSDefn << ";";
                FuncBody->getLocStart().dump(TheRewriter.getSourceMgr());
                llvm::outs() << "\n";
                FR.setBegin(PL);
                TheRewriter.ReplaceText(FR,SSDefn.str());

                // Replace function declaration
                FR = f->getSourceRange();
                param = f->getParamDecl(0);
                SSRepl << "extern void " << FuncName << " (" << param->getOriginalType().getAsString();
                PL = param->getSourceRange().getBegin();
                FR.setEnd(PL);
                TheRewriter.ReplaceText(FR,SSRepl.str());
            }
		}

		return true;
    }

	void flushCtqg (const char* filename) {
		std::string ErrInfo;
		raw_fd_ostream *outfile = new raw_fd_ostream(filename, ErrInfo);
		//outfile.open( filename, std::ofstream::out | std::ofstream::app );
		*outfile << ctqgBuffer.str();
		outfile->close();
	}

private:
    void AddBraces(Stmt *s);

    Rewriter &TheRewriter;
	stringstream ctqgBuffer;
	std::string lastCtqgFunc;
};


// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer
{
public:
    MyASTConsumer(Rewriter &R)
        : Visitor(R)
    {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end();
             b != e; ++b)
            // Traverse the declaration using our AST visitor.
            Visitor.TraverseDecl(*b);
        return true;
    }

	void flushCtqg(const char* filename) {
		Visitor.flushCtqg(filename);
	}

private:
    MyASTVisitor Visitor;
};


int main(int argc, char *argv[])
{
    if (argc != 2) {
        llvm::errs() << "Usage: rewritersample <filename>\n";
        return 1;
    }

    // CompilerInstance will hold the instance of the Clang compiler for us,
    // managing the various objects needed to run the compiler.
    CompilerInstance TheCompInst;
    TheCompInst.createDiagnostics(0, 0);

    // Initialize target info with the default triple for our platform.
    TargetOptions TO;
    TO.Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *TI = TargetInfo::CreateTargetInfo(
        TheCompInst.getDiagnostics(), TO);
    TheCompInst.setTarget(TI);

    TheCompInst.createFileManager();
    FileManager &FileMgr = TheCompInst.getFileManager();
    TheCompInst.createSourceManager(FileMgr);
    SourceManager &SourceMgr = TheCompInst.getSourceManager();
    TheCompInst.createPreprocessor();
    TheCompInst.createASTContext();

    // A Rewriter helps us manage the code rewriting task.
    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

    // Set the main file handled by the source manager to the input file.
    const FileEntry *FileIn = FileMgr.getFile(argv[1]);
    SourceMgr.createMainFileID(FileIn);
    TheCompInst.getDiagnosticClient().BeginSourceFile(
        TheCompInst.getLangOpts(),
        &TheCompInst.getPreprocessor());

    // Create an AST consumer instance which is going to get called by
    // ParseAST.
    MyASTConsumer TheConsumer(TheRewriter);

    // Parse the file to AST, registering our consumer as the AST consumer.
    ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
             TheCompInst.getASTContext());

    // At this point the rewriter's buffer should be full with the rewritten
    // file contents.
    const RewriteBuffer *RewriteBuf =
        TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
    llvm::outs() << string(RewriteBuf->begin(), RewriteBuf->end());

	TheConsumer.flushCtqg("file.ctqg");

    return 0;
}
