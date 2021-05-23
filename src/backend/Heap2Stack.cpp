#include "../core/Team3Passes.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/MemoryBuiltins.h"

#include <vector>

#define MAX_SIZE 1024

using namespace std;
using namespace llvm;

namespace backend {

  bool checkConv(Function* F, set<string>& mallocLikeFunc) {
    for(auto U : F->users()) {
      auto I = dyn_cast<Instruction>(U);
      if(I){
        Function* caller = I->getFunction();
        if(caller->getNumUses() == 0) {
          
        }
        else {
          bool check = false;
          for(auto U : I->users()) {
            if(auto DD = dyn_cast<ReturnInst>(U)) {
              check = checkConv(caller, mallocLikeFunc);
            }
          }
          if(!check)
            return false;
        }        
      }
    }
    mallocLikeFunc.insert(F->getName().str());
    return true;
  }
  
PreservedAnalyses Heap2Stack::run(Function &F, FunctionAnalysisManager &FAM) {
  auto& TLI = FAM.getResult<TargetLibraryAnalysis>(F);
  auto& DL = F.getParent() -> getDataLayout();
  vector<Instruction*> heap_allocation;
  bool forced = F.getNumUses() <= 0;
  
  for(auto& BB : F) {
    for(auto& I : BB) {
      // find heap allocation(malloc) / deallocation(free)
      if(CallInst* call_inst = dyn_cast<CallInst>(&I)) {
        if(Function *callee = call_inst -> getCalledFunction()) {
          if(callee -> getName().str() == "malloc") {
            // found malloc
            if(F.getNumUses()<=0)
              heap_allocation.push_back(call_inst);
            else {
              for(auto U : I.users()) {
                if(auto DD = dyn_cast<ReturnInst>(U)) {
                  if(checkConv(&F, mallocLikeFunc)) {
                    heap_allocation.push_back(call_inst);
                    break;
                  }
                }
                else if(auto I = dyn_cast<BitCastInst>(U)) {
                  for(auto U : I->users()) {
                    if(auto DD = dyn_cast<ReturnInst>(U)) {
                      if(checkConv(&F, mallocLikeFunc)) {
                        heap_allocation.push_back(call_inst);
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  // replace malloc with alloca
  IRBuilder<> IB(F.getContext());
  for(Instruction* I : heap_allocation) {
    IB.SetInsertPoint(dyn_cast<Instruction>(I -> getNextNode()));
    auto* malloc = dyn_cast<CallInst>(I);
    auto* type = getMallocAllocatedType(malloc, &TLI);
    Value* size = getMallocArraySize(malloc, DL, &TLI, true);
    Value* Alloca = IB.CreateBitCast(IB.CreateAlloca(type, size), I -> getType());
    I -> replaceAllUsesWith(Alloca);
    I -> eraseFromParent();
  }
  return PreservedAnalyses::all();
}
}