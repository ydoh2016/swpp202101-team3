#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace std;

namespace backend {
    
PreservedAnalyses LoopUnrollingPass::run(Module &M, ModuleAnalysisManager &MAM) {
    

    return PreservedAnalyses::all();
}
}