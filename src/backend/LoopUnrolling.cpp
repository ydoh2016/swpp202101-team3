#include "../core/Team3Passes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <cassert>

using namespace llvm;
using namespace std;

namespace backend {
    
PreservedAnalyses LoopUnrollingPass::run(Function &F, FunctionAnalysisManager &FAM) {
    auto &LI = FAM.getResult<LoopAnalysis>(F);

    bool changed = false;

    for(auto &L : LI) {
        // If there are multiple exiting blocks but one of them is the latch,
        // Modify latch's terminator operation
        // Otherwise insit on a single exiting block for modifying terminate operation
        BasicBlock *ExitingBlock = L->getLoopLatch();
        if(!ExitingBlock || !L->isLoopExiting(ExitingBlock))
            L->getExitingBlock();
        
        if(ExitingBlock)
        {
            auto *TermIns = dyn_cast<BranchInst>(ExitingBlock->getTerminator());
            
            if(TermIns)
            {
                if(!(TermIns->isConditional()))
                    continue;
                IRBuilder<> builder(TermIns);
                auto *CondVal = TermIns->getCondition();
                auto *NewCondVal = builder.CreateNot(CondVal);
                auto *lhs = TermIns->getSuccessor(0);
                auto *rhs = TermIns->getSuccessor(1);
                auto *NewInst = BranchInst::Create(rhs, lhs, NewCondVal);
                ReplaceInstWithInst(TermIns, NewInst);
                changed = true;
            }
            else
            {
                auto *TermIns2 = dyn_cast<SwitchInst>(ExitingBlock->getTerminator());

                assert(TermIns2);

                if(TermIns2->getNumSuccessors() != 2)
                    continue;
                
                IRBuilder<> builder(TermIns2);
                auto *CondVal = TermIns2->getCondition();
                auto *NewCondVal = builder.CreateICmpEQ(CondVal, ConstantInt::get(Type::getInt32Ty(TermIns2->getContext()), 1, true));
                auto *lhs = TermIns2->getDefaultDest();
                auto *rhs = TermIns2->getSuccessor(1);
                auto *NewInst = BranchInst::Create(lhs, rhs, NewCondVal);
                ReplaceInstWithInst(TermIns2, NewInst);
                changed = true;
            }

            
        }
    }

    if(!changed){
        return PreservedAnalyses::all();
    }

    return getLoopPassPreservedAnalyses();
}
}