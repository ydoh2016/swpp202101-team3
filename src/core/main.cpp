#include "Team3Passes.h"
#include "../backend/Backend.h"
#include "../backend/AddressArgCast.h"
#include "../backend/ConstExprRemove.h"
#include "../backend/GEPUnpack.h"
#include "../backend/RegisterSpill.h"
#include "../backend/UnfoldVectorInst.h"
#include "../backend/ConstantFolding.h"
#include "../backend/SplitSelfLoop.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"

//add header for Dead argument elimination
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
//add header for Dead code elimination
#include "llvm/Transforms/Scalar/ADCE.h"
//add header for Tail call elimination
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"
// add header for gvn
#include "llvm/Transforms/Scalar/GVN.h"

#include <string>

using namespace std;
using namespace llvm;
using namespace backend;

//change arguments for main
// from bin/sf-compiler src.ll output.s 
// to bin/sf-compiler src.ll output.s (pass-name:optional) (outputDbgfile:optional)
//Without specification for pass, pass will be 'all'
//Currently adce, dae, licm, simplifyCFG, tailCallElim supported
//outputDbgfile is optimized ir file.

static cl::opt<string> optInput(
    cl::Positional, cl::desc("<input bitcode file>"), cl::Required,
    cl::value_desc("filename"));

static cl::opt<string> optOutput(
    cl::Positional, cl::desc("<output assembly file>"), cl::value_desc("filename"),
    cl::init(""));

static cl::opt<string> specificPass(
    cl::Positional, cl::desc("<select specific pass>"), cl::value_desc("pass_name"),
    cl::init(""));

cl::opt<string> outputDbg ("debug-file", cl::value_desc("filename"), cl::desc("make debug file in <filename>"));

int main(int argc, char *argv[]) {
  //Parse command line arguments
  cl::ParseCommandLineOptions(argc, argv);
  if (optOutput == "")
      optOutput = ".tmp.s";
  if (specificPass == "")
      specificPass = "all";
  if (outputDbg == "")
      outputDbg = "no";
  bool optPrintProgress = false;

  std::transform(specificPass.begin(), specificPass.end(),specificPass.begin(), ::tolower);

  //Parse input LLVM IR module
  LLVMContext Context;
  unique_ptr<Module> M;  

  SMDiagnostic Error;
  M = parseAssemblyFile(optInput, Error, Context);

  //If loading file failed:
  string errMsg;
  raw_string_ostream os(errMsg);
  Error.print("", os);

  if (!M)
    return 1;

  //////////////////////////////////////////////////// ADD FROM HERE
  // execute IR passes
  FunctionPassManager FPM;
  ModulePassManager MPM;

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  PassBuilder PB;

  // register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  // add existing passes
  //add Dead code Elimination
  if(specificPass == "all" || specificPass == "sprint1" || specificPass == "adce")  
    FPM.addPass(ADCEPass());
  //add  Tail call elimination
  if(specificPass == "all" || specificPass == "sprint1" || specificPass == "tailcallelim")  
    FPM.addPass(TailCallElimPass());
  
  FunctionPassManager FPM1;
  FunctionPassManager FPM2;
  //add custom passes
  if(specificPass == "all" || specificPass == "sprint1" || specificPass == "mergebasicblocks")
    FPM1.addPass(MergeBasicBlocksPass());

  //add Dead argument elimination
  if(specificPass == "all" || specificPass == "sprint1" || specificPass == "dae")  
    MPM.addPass(DeadArgumentEliminationPass());
  
  if(specificPass == "all" || specificPass == "sprint1" || specificPass == "constantfolding")  
    FPM2.addPass(ConstantFolding());

  // from FPM to MPM
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM1)));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));
  
  MPM.run(*M, MAM);
  //////////////////////////////////////////////////// BY HERE

  SplitSelfLoopPass().run(*M, MAM);
  UnfoldVectorInstPass().run(*M, MAM);
  LivenessAnalysis().run(*M, MAM);
  SpillCostAnalysis().run(*M, MAM);
  AddressArgCastPass().run(*M, MAM);
  ConstExprRemovePass().run(*M, MAM);
  GEPUnpackPass().run(*M, MAM);
  RegisterSpillPass().run(*M, MAM);
  // use this for debugging
  if(outputDbg != "no") {    
    outs() << "debug output for " + specificPass + " into " + outputDbg + "\n";
    std::error_code ec;
    raw_fd_ostream output(outputDbg, ec);
    output << *M;
    output.close();
  }

  // execute backend to emit assembly
  Backend B(optOutput, optPrintProgress);
  B.run(*M, MAM);
  
  return 0;
}
