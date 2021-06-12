#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

#include <vector>
#include <algorithm>
using namespace llvm;
using namespace std;

namespace backend {

int InliningPass::MAX_BB_COUNT = 5;

int InliningPass::getBBCount(Function *F) {
    int count = 0;
    for (auto &BB : *F) {
        ++count;
    }
    return count;
}

void InliningPass::getFunctionCalls(Module *M, vector<CallInst*> *calls) {
    for (Function &F : *M) {
        for (inst_iterator IT = inst_begin(&F), E = inst_end(&F); IT != E; ++IT) {
            Instruction *I = &*IT;
            CallInst *CI = dyn_cast<CallInst>(I);
            if (CI != nullptr) {
                Function *called = CI->getCalledFunction();
                if (getBBCount(called) <= MAX_BB_COUNT && 
                    called->hasExactDefinition() &&
                    called != &F) { // No inline for recursions
                    calls->push_back(CI);
                }
            }
        }
    }
}

void InliningPass::cloneIntoCaller(CallInst *call, DominatorTree &DT) {
    Function *caller = call->getFunction();
    Function *callee = call->getCalledFunction();

    BasicBlock *BBCaller = call->getParent();
    BasicBlock *BBAfter = SplitBlock(BBCaller, call, &DT);
    int succCount = BBCaller->getTerminator()->getNumSuccessors();
    for (int i = 0; i < succCount; ++i) {
        BasicBlock *BBSucc = BBCaller->getTerminator()->getSuccessor(i);
        BBSucc->replacePhiUsesWith(BBCaller, BBAfter);
    }

    BasicBlock *BBCopiedEntry = nullptr; // The inlined entry block
    PHINode *phiInserted = nullptr; // To prevent multiple phi node insertion
    
    vector<BasicBlock*> BBCallees; // To prevent insertions during the range-based for
    for (auto &BBCallee : *callee) {
        BBCallees.push_back(&BBCallee);
    }

    map<Value*, Value*> instructionMap;
    map<BasicBlock*, BasicBlock*> BBMap;
    // Copy the inilined function's BBs into the caller function
    // BB: original blocks
    for (auto BBCallee : BBCallees) {
        ValueToValueMapTy VM;
        BasicBlock *BBCopied = CloneBasicBlock(BBCallee, VM, "", caller);
        BBMap.insert({BBCallee, BBCopied});

        // Cache the copied entry block
        if (BBCallee == &callee->getEntryBlock()) {
            BBCopiedEntry = BBCopied;
        }

        for (auto &I : *BBCallee) {
            Value *from = &I;
            Value *to = VM[&I];
            instructionMap.insert({from, to});
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

    // Remap instructions in the copied basic block
    for (auto mapping : instructionMap) {
        Value *from = mapping.first;
        Value *to = mapping.second;
        for (auto it = from->use_begin(), end = from->use_end(); it != end;) {
            Use &U = *it++;
            User *Usr = U.getUser();
            Instruction *UsrI = dyn_cast<Instruction>(Usr);
            if (UsrI->getFunction() == caller) {
                U.set(to);
            }
        }
    }

    for (auto mapping : BBMap) {
        BasicBlock *BBFrom = mapping.first;
        BasicBlock *BBTo = mapping.second;
        for (auto it = BBFrom->use_begin(), end = BBFrom->use_end(); it != end;) {
            Use &U = *it++;
            User *Usr = U.getUser();
            Instruction *UsrI = dyn_cast<Instruction>(Usr);
            if (UsrI->getFunction() == caller) {
                U.set(BBTo);
            }
        }

        for (auto &phi : BBTo->phis()) {
            int count = phi.getNumIncomingValues();
            for (int i = 0; i < count; ++i) {
                BasicBlock *BBIncoming = phi.getIncomingBlock(i);
                if (BBMap.find(BBIncoming) != BBMap.end()) {
                    phi.setIncomingBlock(i, BBMap[BBIncoming]);
                }
            }
        }
    }

    BBCaller->getTerminator()->setSuccessor(0, BBCopiedEntry);

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
    
    return PreservedAnalyses::none();
}
}

