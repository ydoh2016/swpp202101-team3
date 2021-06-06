#include "../core/Team3Passes.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/MemoryBuiltins.h"

#include <map>

namespace backend {

  Instruction* OptiMemAccess::checkOptiPossible(Value* vn) {
    while(auto v = vn) {
      vn = nullptr;
      if(auto ptInst = dyn_cast<Instruction>(v)) {
        if(auto allocaInst = dyn_cast<AllocaInst>(ptInst)) {
          return ptInst;
        }
        else if(auto callInst = dyn_cast<CallInst>(ptInst)) {
          return nullptr;
        }
        else if(auto bitcastInst = dyn_cast<BitCastInst>(ptInst)) {
          vn = bitcastInst->getOperand(0);
        }
        else if(auto getElemI = dyn_cast<GetElementPtrInst>(ptInst)) {
          vn = getElemI->getPointerOperand();
        }
      }
      else if(auto argV = dyn_cast<Argument>(v)) {
        return nullptr;
      }
      else
        return nullptr;
    }
    return nullptr;
  }

  PreservedAnalyses OptiMemAccess::run(Function& F, FunctionAnalysisManager& FAM) {
    auto& TLI = FAM.getResult<TargetLibraryAnalysis>(F);
    auto& DL = F.getParent()->getDataLayout();

    llvm::DominatorTree dt;
    dt.recalculate(F);
    llvm::LoopInfoBase<llvm::BasicBlock, llvm::Loop> loopChecker;
    loopChecker.releaseMemory();
    loopChecker.analyze(dt);
    
    for(auto& BB : F) {
      // check whether load or store cmd exist
      bool chkMemAccess = false;
      for(auto& I : BB) {
        if(auto inst = dyn_cast<LoadInst>(&I)) {
          auto allocInst = checkOptiPossible(inst->getPointerOperand());
          if(allocInst && (loopChecker.getLoopDepth(&BB) > 0)) {
            optiMemAccMap[inst] = allocInst;
          }
        }
        else if(auto inst = dyn_cast<StoreInst>(&I)) {
          auto allocInst = checkOptiPossible(inst->getPointerOperand());
          if(allocInst && (loopChecker.getLoopDepth(&BB) > 0)) {
            optiMemAccMap[inst] = allocInst;
          }
        }
      }
    }
    return PreservedAnalyses::all();
  }

}