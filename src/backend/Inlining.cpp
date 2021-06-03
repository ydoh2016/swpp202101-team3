#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"

#include <vector>
#include <algorithm>
using namespace llvm;
using namespace std;

namespace backend {

void InliningPass::getFunctionCalls(Module *M, vector<CallInst*> *calls) {
    for (Function &F : *M) {
        for (inst_iterator IT = inst_begin(&F), E = inst_end(&F); IT != E; ++IT) {
            Instruction *I = &*IT;
            CallInst *CI = dyn_cast<CallInst>(I);
            if (CI != nullptr) {
                calls->push_back(CI);
            }
        }
    }
}

void InliningPass::cloneIntoCaller(CallInst *call) {
    Function *caller = call->getFunction();
    Function *callee = call->getCalledFunction();
    divideBasicBlock(call);
}

void InliningPass::divideBasicBlock(Instruction *criteria) {
    Function *caller = criteria->getFunction();
    BasicBlock *BBCaller = criteria->getParent();

    ValueToValueMapTy VMAfter;
    BasicBlock *BBAfter = CloneBasicBlock(BBCaller, VMAfter, "", caller);

    vector<Instruction*> IAfter;
    bool callInstSeen = false;

    for (Instruction &I : *BBAfter) {
        if (dyn_cast<Instruction>(&I) == criteria) {
            callInstSeen = true;
        }
        if (callInstSeen) {
            IAfter.push_back(&I);
        }
    }

    for (Instruction *I : IAfter) {
        I->eraseFromParent();
    }
}

PreservedAnalyses InliningPass::run(Module &M, ModuleAnalysisManager &MAM) {
    vector<CallInst*> functionCalls;
    getFunctionCalls(&M, &functionCalls);
    for (auto call : functionCalls) {
        cloneIntoCaller(call);
    }

    return PreservedAnalyses::all();
}
}

