#include "Team3Passes.h"
#include "../backend/Backend.h"
#include "../backend/AddressArgCast.h"
#include "../backend/ConstExprRemove.h"
#include "../backend/GEPUnpack.h"
#include "../backend/RegisterSpill.h"
#include "../backend/UnfoldVectorInst.h"
#include "../backend/SplitSelfLoop.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"

//add header for Dead argument elimination
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
//add header for Dead code elimination
#include "llvm/Transforms/Scalar/ADCE.h"
//add header for Branch-related optimizations including br -> switch
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
//add header for Loop invariant code motion
#include "llvm/Transforms/Scalar/LICM.h"
//add header for Tail call elimination
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"

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

int main(int argc, char *argv[]) {
  //Parse command line arguments
  if(argc < 3) return -1;
  string optInput = argv[1];
  string optOutput = argv[2];
  string specificPass = argc > 3 ? argv[3] : "all";
  string outputDbg = argc > 4 ? argv[4] : "no";
  std::transform(specificPass.begin(), specificPass.end(),specificPass.begin(), ::tolower);
  
  bool optPrintProgress = false;

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
  if(specificPass == "all" || specificPass == "adce")  
    FPM.addPass(ADCEPass());
  //add Branch-related optimizations including br -> switch
  if(specificPass == "all" || specificPass == "simplifycfg")  
    FPM.addPass(SimplifyCFGPass());
  //add Loop invariant code motion
  if(specificPass == "all" || specificPass == "licm")  
    FPM.addPass(createFunctionToLoopPassAdaptor(LICMPass()));
  //add  Tail call elimination
  if(specificPass == "all" || specificPass == "tailcallelim")  
    FPM.addPass(TailCallElimPass());
  //add Dead argument elimination
  
  FunctionPassManager FPM1;
  FunctionPassManager FPM2;
  FunctionPassManager FPM3;

  //add custom passes
  if(specificPass == "all" || specificPass == "mergebasicblocks")
    FPM1.addPass(MergeBasicBlocksPass());

  if(specificPass == "all" || specificPass == "dae")  
    MPM.addPass(DeadArgumentEliminationPass());
  
  if(specificPass == "all" || specificPass == "constantfolding")  
    FPM2.addPass(ConstantFolding());

  if(specificPass == "all" || specificPass == "sprint2" || specificPass == "heap2stack")
    FPM3.addPass(Heap2Stack());


  // from FPM to MPM
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM1)));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM3)));
  
  
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
