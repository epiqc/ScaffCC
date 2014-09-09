#include <iostream>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/ParseAST.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/AST/ASTContext.h"

int main(int argc, char *argv[]) {
	using clang::CompilerInstance;
	using clang::TargetOptions;
	using clang::TargetInfo;
	using clang::FileEntry;
	using clang::ASTConsumer;
	using clang::DiagnosticOptions;
	using clang::TextDiagnosticPrinter;

	CompilerInstance CI;
	DiagnosticOptions diagnosticOptions;
	TextDiagnosticPrinter *pTextDiagnosticPrinter =
		new TextDiagnosticPrinter(
			llvm::outs(), diagnosticOptions, true);
	CI.createDiagnostics(0, NULL, pTextDiagnosticPrinter);

	TargetOptions targetOptions;
	targetOptions.Triple = llvm::sys::getDefaultTargetTriple();
	TargetInfo *pTargetInfo = TargetInfo::CreateTargetInfo(
			CI.getDiagnostics(), targetOptions);
	CI.setTarget(pTargetInfo);

	CI.createFileManager();
	CI.createSourceManager(CI.getFileManager());
	CI.createPreprocessor();
	CI.getPreprocessorOpts().UsePredefines = false;
	ASTConsumer *astConsumer = new ASTConsumer();
	CI.setASTConsumer(astConsumer);

	CI.createASTContext();
	CI.createSema(clang::TU_Complete, NULL);

	const FileEntry *pFile = CI.getFileManager().getFile(argv[1]);
	CI.getSourceManager().createMainFileID(pFile);

	CI.getASTContext().BuiltinInfo.InitializeBuiltins(
		CI.getPreprocessor().getIdentifierTable(), CI.getLangOpts());

	CI.getDiagnosticClient().BeginSourceFile(
			CI.getLangOpts(), &(CI.getPreprocessor()));
	clang::ParseAST(CI.getSema());
	CI.getDiagnosticClient().EndSourceFile();

	return 0;
	}
