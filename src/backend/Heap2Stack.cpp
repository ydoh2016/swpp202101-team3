#include "../core/Team3Passes.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/MemoryBuiltins.h"

#include <vector>

#define MAX_SIZE 1024

using namespace std;
using namespace llvm;

namespace backend {
PreservedAnalyses Heap2Stack::run(Function &F, FunctionAnalysisManager &FAM) {
  auto& TLI = FAM.getResult<TargetLibraryAnalysis>(F);
  auto& DL = F.getParent() -> getDataLayout();
  vector<Instruction*> heap_allocation;
  // find malloc
  for(auto& BB : F) {
    for(auto& I : BB) {
      // find heap allocation(malloc) / deallocation(free)
      if(CallInst* call_inst = dyn_cast<CallInst>(&I)) {
        if(Function *callee = call_inst -> getCalledFunction()) {
          if(callee -> getName().str() == "malloc") {
            // found malloc
            heap_allocation.push_back(call_inst);
            //BasicBlock::iterator bb_it(I);
            //ReplaceInstWithInst(I.getParent() -> getInstList(), bb_it, createAllocaInstAtEntry(BB));
          }
        }
      }
    }
  }
  // replace malloc and free with alloca
  IRBuilder<> IB(F.getContext());
  for(Instruction* I : heap_allocation) {
    IB.SetInsertPoint(dyn_cast<Instruction>(F.getEntryBlock().getFirstInsertionPt()));
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