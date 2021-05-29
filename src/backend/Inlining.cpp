#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"

#include <vector>
#include <algorithm>

void InliningPass::getFunctionCalls(vector<CallInst> *calls) {

}

PreservedAnalyses InliningPass::run(Module &M, ModuleAnalysisManager &MAM) {
    return PreservedAnalyses::all();
}
