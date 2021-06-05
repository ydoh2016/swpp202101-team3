#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

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
            if (CI != nullptr && 
                CI->getCalledFunction()->hasExactDefinition() &&
                CI->getCalledFunction() != &F) { // No inline for recursions
                calls->push_back(CI);
            }
        }
    }
}

void InliningPass::cloneIntoCaller(CallInst *call, DominatorTree &DT) {
    Function *caller = call->getFunction();
    Function *callee = call->getCalledFunction();

    BasicBlock *BBCaller = call->getParent();
    BasicBlock *BBAfter = SplitBlock(BBCaller, call, &DT);

    BasicBlock *BBEntry; // The inlined entry block
    PHINode *phiInserted = nullptr; // To prevent multiple phi node insertion

    vector<BasicBlock*> BBs; // To prevent insertions during the range-based for 
    for (auto &BB : *callee) {
        BBs.push_back(&BB);
    }

    // Copy the inilined function's BBs into the caller function
    // BB: original blocks
    for (auto BB : BBs) {
        ValueToValueMapTy VM;
        BasicBlock *BBCopied = CloneBasicBlock(BB, VM, "", caller);

        // Cache the copied entry block
        if (BB == &callee->getEntryBlock()) {
            BBEntry = BBCopied;
        }

        // Remap instructions in the copied basic block
        for (auto &I : *BB) {
            Value *to = VM[&I];
            for (auto it = I.use_begin(), end = I.use_end(); it != end;) {
                Use &U = *it++;
                User *Usr = U.getUser();
                Instruction *UsrI = dyn_cast<Instruction>(Usr);
                if (UsrI->getParent() == BBCopied) {
                    U.set(to);
                }
            }
        }

        // Remap basic block uses in the caller to the copied one
        for (auto it = BB->use_begin(), end = BB->use_end(); it != end;) {
            Use &U = *it++;
            User *Usr = U.getUser();
            Instruction *UsrI = dyn_cast<Instruction>(Usr);
            if (UsrI->getFunction() == caller) {
                U.set(BBCopied);
            }
        }

        int argCount = call->getNumArgOperands();
        for (int i = 0; i < argCount; ++i) {
            Value *VArg = call->getArgOperand(i); // i'th argument to the inlined function
            Value *VParam = callee->getArg(i); // i'th parameter of the inlined function
            // Replace all uses of parameters in the copied block into the VArg
            for (auto it = VParam->use_begin(), end = VParam->use_end(); it != end;) {
                Use &U = *it++;
                User *Usr = U.getUser();
                Instruction *UsrI = dyn_cast<Instruction>(Usr);
                if (UsrI->getParent() == BBCopied) {
                    U.set(VArg);
                }
            }
        }
        
        Instruction *terminator = BBCopied->getTerminator();
        // If this block is a return block
        if (dyn_cast<ReturnInst>(terminator) != nullptr) {
            // If this doesn't return void
            if (terminator->getNumOperands() != 0) {
                Value *VReturn = terminator->getOperand(0);
                // Create a new phi node
                if (phiInserted == nullptr) {
                    phiInserted = PHINode::Create(call->getType(), 1, "");
                    //Insert before the first non-phi instruction
                    Instruction *insertPoint = BBAfter->getFirstNonPHI();
                    phiInserted->insertBefore(insertPoint);
                }
                phiInserted->addIncoming(VReturn, BBCopied);
            }
            terminator->eraseFromParent();
            BranchInst::Create(BBAfter, BBCopied);
        }
    }

    BBCaller->getTerminator()->setSuccessor(0, BBEntry);

    if (phiInserted != nullptr) {
        call->replaceAllUsesWith(phiInserted);
    }
    call->eraseFromParent();
}

PreservedAnalyses InliningPass::run(Module &M, ModuleAnalysisManager &MAM) {
    vector<CallInst*> functionCalls;
    getFunctionCalls(&M, &functionCalls);

    for (auto call : functionCalls) {
        FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
        DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(*call->getFunction());
        cloneIntoCaller(call, DT);
    }

    return PreservedAnalyses::all();
}
}

