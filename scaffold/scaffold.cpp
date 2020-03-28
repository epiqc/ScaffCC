 #include <iostream>
#include <memory>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/AST/ASTContext.h"

int main(int argc, char *argv[]) {
	using clang::CompilerInstance;
	using clang::TargetOptions;
	using clang::TargetInfo;
	using clang::FileEntry;
	using clang::ASTConsumer;
	using clang::DiagnosticOptions;
	using clang::TextDiagnosticPrinter;
	using clang::LangOptions;
	using clang::FileID;

	CompilerInstance CI;
	DiagnosticOptions diagnosticOptions;
	TextDiagnosticPrinter *pTextDiagnosticPrinter =
		new TextDiagnosticPrinter(llvm::outs(), &diagnosticOptions, true);
	CI.createDiagnostics(0, NULL, pTextDiagnosticPrinter);

	TargetOptions targetOptions;
	const std::shared_ptr<TargetOptions> optPointer = 
		std::make_shared<TargetOptions>(targetOptions);
	targetOptions.Triple = llvm::sys::getDefaultTargetTriple();
	TargetInfo *pTargetInfo = TargetInfo::CreateTargetInfo(
			CI.getDiagnostics(), optPointer);
	CI.setTarget(pTargetInfo);

	CI.createFileManager();
	CI.createSourceManager(CI.getFileManager());
	CI.createPreprocessor(clang::TU_Complete);
	// Check this ^
	CI.getPreprocessorOpts().UsePredefines = false;
	ASTConsumer astConsumer = ASTConsumer();
	CI.setASTConsumer(llvm::make_unique<ASTConsumer>(astConsumer));

	CI.createASTContext();
	CI.createSema(clang::TU_Complete, NULL);

	const FileEntry *pFile = CI.getFileManager().getFile(argv[1]);
	CI.getSourceManager().setMainFileID(CI.getSourceManager().createFileID(pFile,
		clang::SourceLocation(), clang::SrcMgr::C_User));

	CI.getASTContext().BuiltinInfo.initializeBuiltins(
		CI.getPreprocessor().getIdentifierTable(), CI.getLangOpts());

	CI.getDiagnosticClient().BeginSourceFile(
			CI.getLangOpts(), &(CI.getPreprocessor()));
	clang::ParseAST(CI.getSema());
	CI.getDiagnosticClient().EndSourceFile();

	return 0;
	}
