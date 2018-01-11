//===- dsymutil.cpp - Debug info dumping utility for llvm -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This program is a utility that aims to be a dropin replacement for
// Darwin's dsymutil.
//
//===----------------------------------------------------------------------===//

#include "dsymutil.h"
#include "CFBundle.h"
#include "DebugMap.h"
#include "MachOUtils.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Object/MachO.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ThreadPool.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/thread.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <system_error>

using namespace llvm;
using namespace llvm::cl;
using namespace llvm::dsymutil;

static OptionCategory DsymCategory("Specific Options");
static opt<bool> Help("h", desc("Alias for -help"), Hidden);
static opt<bool> Version("v", desc("Alias for -version"), Hidden);

static list<std::string> InputFiles(Positional, OneOrMore,
                                    desc("<input files>"), cat(DsymCategory));

static opt<std::string>
    OutputFileOpt("o",
                  desc("Specify the output file. default: <input file>.dwarf"),
                  value_desc("filename"), cat(DsymCategory));

static opt<std::string> OsoPrependPath(
    "oso-prepend-path",
    desc("Specify a directory to prepend to the paths of object files."),
    value_desc("path"), cat(DsymCategory));

static opt<bool> DumpStab(
    "symtab",
    desc("Dumps the symbol table found in executable or object file(s) and\n"
         "exits."),
    init(false), cat(DsymCategory));
static alias DumpStabA("s", desc("Alias for --symtab"), aliasopt(DumpStab));

static opt<bool> FlatOut("flat",
                         desc("Produce a flat dSYM file (not a bundle)."),
                         init(false), cat(DsymCategory));
static alias FlatOutA("f", desc("Alias for --flat"), aliasopt(FlatOut));

static opt<unsigned> NumThreads(
    "num-threads",
    desc("Specifies the maximum number (n) of simultaneous threads to use\n"
         "when linking multiple architectures."),
    value_desc("n"), init(0), cat(DsymCategory));
static alias NumThreadsA("j", desc("Alias for --num-threads"),
                         aliasopt(NumThreads));

static opt<bool> Verbose("verbose", desc("Verbosity level"), init(false),
                         cat(DsymCategory));

static opt<bool>
    NoOutput("no-output",
             desc("Do the link in memory, but do not emit the result file."),
             init(false), cat(DsymCategory));

static opt<bool>
    NoTimestamp("no-swiftmodule-timestamp",
                desc("Don't check timestamp for swiftmodule files."),
                init(false), cat(DsymCategory));

static list<std::string> ArchFlags(
    "arch",
    desc("Link DWARF debug information only for specified CPU architecture\n"
         "types. This option can be specified multiple times, once for each\n"
         "desired architecture. All CPU architectures will be linked by\n"
         "default."), value_desc("arch"),
    ZeroOrMore, cat(DsymCategory));

static opt<bool>
    NoODR("no-odr",
          desc("Do not use ODR (One Definition Rule) for type uniquing."),
          init(false), cat(DsymCategory));

static opt<bool> DumpDebugMap(
    "dump-debug-map",
    desc("Parse and dump the debug map to standard output. Not DWARF link "
         "will take place."),
    init(false), cat(DsymCategory));

static opt<bool> InputIsYAMLDebugMap(
    "y", desc("Treat the input file is a YAML debug map rather than a binary."),
    init(false), cat(DsymCategory));

static bool createPlistFile(llvm::StringRef Bin, llvm::StringRef BundleRoot) {
  if (NoOutput)
    return true;

  // Create plist file to write to.
  llvm::SmallString<128> InfoPlist(BundleRoot);
  llvm::sys::path::append(InfoPlist, "Contents/Info.plist");
  std::error_code EC;
  llvm::raw_fd_ostream PL(InfoPlist, EC, llvm::sys::fs::F_Text);
  if (EC) {
    llvm::errs() << "error: cannot create plist file " << InfoPlist << ": "
                 << EC.message() << '\n';
    return false;
  }

  CFBundleInfo BI = getBundleInfo(Bin);

  if (BI.IDStr.empty()) {
    llvm::StringRef BundleID = *llvm::sys::path::rbegin(BundleRoot);
    if (llvm::sys::path::extension(BundleRoot) == ".dSYM")
      BI.IDStr = llvm::sys::path::stem(BundleID);
    else
      BI.IDStr = BundleID;
  }

  // Print out information to the plist file.
  PL << "<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n"
     << "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
     << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
     << "<plist version=\"1.0\">\n"
     << "\t<dict>\n"
     << "\t\t<key>CFBundleDevelopmentRegion</key>\n"
     << "\t\t<string>English</string>\n"
     << "\t\t<key>CFBundleIdentifier</key>\n"
     << "\t\t<string>com.apple.xcode.dsym." << BI.IDStr << "</string>\n"
     << "\t\t<key>CFBundleInfoDictionaryVersion</key>\n"
     << "\t\t<string>6.0</string>\n"
     << "\t\t<key>CFBundlePackageType</key>\n"
     << "\t\t<string>dSYM</string>\n"
     << "\t\t<key>CFBundleSignature</key>\n"
     << "\t\t<string>\?\?\?\?</string>\n";

  if (!BI.OmitShortVersion())
    PL << "\t\t<key>CFBundleShortVersionString</key>\n"
       << "\t\t<string>" << BI.ShortVersionStr << "</string>\n";

  PL << "\t\t<key>CFBundleVersion</key>\n"
     << "\t\t<string>" << BI.VersionStr << "</string>\n"
     << "\t</dict>\n"
     << "</plist>\n";

  PL.close();
  return true;
}

static bool createBundleDir(llvm::StringRef BundleBase) {
  if (NoOutput)
    return true;

  llvm::SmallString<128> Bundle(BundleBase);
  llvm::sys::path::append(Bundle, "Contents", "Resources", "DWARF");
  if (std::error_code EC = create_directories(Bundle.str(), true,
                                              llvm::sys::fs::perms::all_all)) {
    llvm::errs() << "error: cannot create directory " << Bundle << ": "
                 << EC.message() << "\n";
    return false;
  }
  return true;
}

static std::string getOutputFileName(llvm::StringRef InputFile) {
  if (FlatOut) {
    // If a flat dSYM has been requested, things are pretty simple.
    if (OutputFileOpt.empty()) {
      if (InputFile == "-")
        return "a.out.dwarf";
      return (InputFile + ".dwarf").str();
    }

    return OutputFileOpt;
  }

  // We need to create/update a dSYM bundle.
  // A bundle hierarchy looks like this:
  //   <bundle name>.dSYM/
  //       Contents/
  //          Info.plist
  //          Resources/
  //             DWARF/
  //                <DWARF file(s)>
  std::string DwarfFile =
      InputFile == "-" ? llvm::StringRef("a.out") : InputFile;
  llvm::SmallString<128> BundleDir(OutputFileOpt);
  if (BundleDir.empty())
    BundleDir = DwarfFile + ".dSYM";
  if (!createBundleDir(BundleDir) || !createPlistFile(DwarfFile, BundleDir))
    return "";

  llvm::sys::path::append(BundleDir, "Contents", "Resources", "DWARF",
                          llvm::sys::path::filename(DwarfFile));
  return BundleDir.str();
}

static Expected<sys::fs::TempFile> createTempFile() {
  llvm::SmallString<128> TmpModel;
  llvm::sys::path::system_temp_directory(true, TmpModel);
  llvm::sys::path::append(TmpModel, "dsym.tmp%%%%%.dwarf");
  return sys::fs::TempFile::create(TmpModel);
}

namespace {
struct TempFileVector {
  std::vector<sys::fs::TempFile> Files;
  ~TempFileVector() {
    for (sys::fs::TempFile &Tmp : Files) {
      if (Error E = Tmp.discard())
        errs() << toString(std::move(E));
    }
  }
};
} // namespace

int main(int argc, char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram StackPrinter(argc, argv);
  llvm::llvm_shutdown_obj Shutdown;
  LinkOptions Options;
  void *P = (void *)(intptr_t)getOutputFileName;
  std::string SDKPath = llvm::sys::fs::getMainExecutable(argv[0], P);
  SDKPath = llvm::sys::path::parent_path(SDKPath);

  HideUnrelatedOptions(DsymCategory);
  llvm::cl::ParseCommandLineOptions(
      argc, argv,
      "manipulate archived DWARF debug symbol files.\n\n"
      "dsymutil links the DWARF debug information found in the object files\n"
      "for the executable <input file> by using debug symbols information\n"
      "contained in its symbol table.\n");

  if (Help) {
    PrintHelpMessage();
    return 0;
  }

  if (Version) {
    llvm::cl::PrintVersionMessage();
    return 0;
  }

  Options.Verbose = Verbose;
  Options.NoOutput = NoOutput;
  Options.NoODR = NoODR;
  Options.NoTimestamp = NoTimestamp;
  Options.PrependPath = OsoPrependPath;

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllTargets();
  llvm::InitializeAllAsmPrinters();

  if (!FlatOut && OutputFileOpt == "-") {
    llvm::errs() << "error: cannot emit to standard output without --flat\n";
    return 1;
  }

  if (InputFiles.size() > 1 && FlatOut && !OutputFileOpt.empty()) {
    llvm::errs() << "error: cannot use -o with multiple inputs in flat mode\n";
    return 1;
  }

  for (const auto &Arch : ArchFlags)
    if (Arch != "*" && Arch != "all" &&
        !llvm::object::MachOObjectFile::isValidArch(Arch)) {
      llvm::errs() << "error: Unsupported cpu architecture: '" << Arch << "'\n";
      return 1;
    }

  for (auto &InputFile : InputFiles) {
    // Dump the symbol table for each input file and requested arch
    if (DumpStab) {
      if (!dumpStab(InputFile, ArchFlags, OsoPrependPath))
        return 1;
      continue;
    }

    auto DebugMapPtrsOrErr = parseDebugMap(InputFile, ArchFlags, OsoPrependPath,
                                           Verbose, InputIsYAMLDebugMap);

    if (auto EC = DebugMapPtrsOrErr.getError()) {
      llvm::errs() << "error: cannot parse the debug map for \"" << InputFile
                   << "\": " << EC.message() << '\n';
      return 1;
    }

    if (DebugMapPtrsOrErr->empty()) {
      llvm::errs() << "error: no architecture to link\n";
      return 1;
    }

    if (NumThreads == 0)
      NumThreads = llvm::thread::hardware_concurrency();
    if (DumpDebugMap || Verbose)
      NumThreads = 1;
    NumThreads = std::min<unsigned>(NumThreads, DebugMapPtrsOrErr->size());


    // If there is more than one link to execute, we need to generate
    // temporary files.
    bool NeedsTempFiles = !DumpDebugMap && (*DebugMapPtrsOrErr).size() != 1;
    llvm::SmallVector<MachOUtils::ArchAndFilename, 4> TempFiles;
    TempFileVector TempFileStore;
    for (auto &Map : *DebugMapPtrsOrErr) {
      if (Verbose || DumpDebugMap)
        Map->print(llvm::outs());

      if (DumpDebugMap)
        continue;

      if (Map->begin() == Map->end())
        llvm::errs() << "warning: no debug symbols in executable (-arch "
                     << MachOUtils::getArchName(Map->getTriple().getArchName())
                     << ")\n";

      std::string OutputFile = getOutputFileName(InputFile);
      std::unique_ptr<raw_fd_ostream> OS;
      if (NeedsTempFiles) {
        Expected<sys::fs::TempFile> T = createTempFile();
        if (!T) {
          errs() << toString(T.takeError());
          return 1;
        }
        OS = llvm::make_unique<raw_fd_ostream>(T->FD, /*shouldClose*/ false);
        OutputFile = T->TmpName;
        TempFileStore.Files.push_back(std::move(*T));
      } else {
        std::error_code EC;
        OS = llvm::make_unique<raw_fd_ostream>(NoOutput ? "-" : OutputFile, EC,
                                         sys::fs::F_None);
        if (EC) {
          errs() << OutputFile << ": " << EC.message();
          return 1;
        }
      }

      std::atomic_char AllOK(1);
      auto LinkLambda = [&]() {
        AllOK.fetch_and(linkDwarf(*OS, *Map, Options));
      };

      // FIXME: The DwarfLinker can have some very deep recursion that can max
      // out the (significantly smaller) stack when using threads. We don't
      // want this limitation when we only have a single thread.
      if (NumThreads == 1) {
        LinkLambda();
      } else {
        llvm::ThreadPool Threads(NumThreads);
        Threads.async(LinkLambda);
        Threads.wait();
      }
      if (!AllOK)
        return 1;

      if (NeedsTempFiles)
        TempFiles.emplace_back(Map->getTriple().getArchName().str(),
                               OutputFile);
    }


    if (NeedsTempFiles &&
        !MachOUtils::generateUniversalBinary(
            TempFiles, getOutputFileName(InputFile), Options, SDKPath))
      return 1;
  }

  return 0;
}
