#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"

#include <vector>
#include <algorithm>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {
int AbbrMemPass::MAXSEQ = 8;

vector<Instruction*> AbbrMemPass::getInst(BasicBlock *BB, unsigned opcode) {
    vector<Instruction*> instList;
    for (auto &I : *BB) {
        if (I.getOpcode() == opcode) {
            instList.push_back(&I);
            //outs() << "I: " << I.getName() << "\n";
        }
    }
    return instList;
}


void AbbrMemPass::getSequence(const vector<Instruction*> &instList) {
    vector<vector<Value*>> sequences;
    vector<Value*> sequence;
    int sz = instList.size();
    vector<bool> isRemaining(sz, true); // Checks if an instruction was added to a sequence

    for (int i = 0; i < sz; ++i) {
        if (!isRemaining[i]) {
            continue;
        }

        Instruction *I1 = instList[i];
        sequence = vector<Value*>(MAXSEQ);
        sequence[0] = I1;

        for (int j = i + 1; j < sz; ++j) {
            if (!isRemaining[j]) {
                continue;
            }
            Instruction *I2 = instList[j];
            int i = 0;
            int *diff = &i;
            bool isSequence = inSameSequence(I1, I2, diff);
            if (isSequence && *diff < MAXSEQ) {
                sequence[*diff] = I2;
                isRemaining[j] = false;
            } else {
                continue;
            }
        }

        if (count(sequence.begin(), sequence.end(), nullptr) <= MAXSEQ - 2) {
            sequences.push_back(sequence);
        }
    }
}


bool AbbrMemPass::isIdentical(Value *V1, Value *V2) {
    if (isa<ConstantInt>(V1) && isa<ConstantInt>(V2)) {
        ConstantInt *C1 = dyn_cast<ConstantInt>(V1);
        ConstantInt *C2 = dyn_cast<ConstantInt>(V2);
        return C1->getZExtValue() == C2->getZExtValue();
    }

    if (V1->getName() == V2->getName()) {
        return true;
    }

    if (isa<Instruction>(V1) && isa<Instruction>(V2)) {
        auto I1 = dyn_cast<Instruction>(V1);
        auto I2 = dyn_cast<Instruction>(V2);
        if (I1->getOpcode() != I2->getOpcode()) {
            return false;
        }
        if (I1->getOpcode() == Instruction::PHI) {
            return false;
        }
        bool result = true;
        int count = I1->getNumOperands();
        for (int i = 0; i < count; ++i) {
            auto OPD1 = I1->getOperand(i);
            auto OPD2 = I2->getOperand(i);
            result &= isIdentical(OPD1, OPD2);
        }
        return result;
    } else {
        return false;
    }
}


bool AbbrMemPass::inSameSequence(Value *V1, Value *V2, int *difference) {
    // Load instructions
    auto I1 = dyn_cast<Instruction>(V1);
    auto I2 = dyn_cast<Instruction>(V2);

    assert (I1 && I2 && "I1 and I2 must be instructions");

    // getelementptr instructions
    auto GP1 = dyn_cast<Instruction>(I1->getOperand(0));
    auto GP2 = dyn_cast<Instruction>(I2->getOperand(0));

    // The pointer from each getelementptr
    auto P1 = GP1->getOperand(0);
    auto P2 = GP2->getOperand(0);
    // They should be the same
    if (P1 != P2) { 
        return false;
    }

    // The instruction for index from each getelementptr
    auto IDX1 = dyn_cast<Instruction>(GP1->getOperand(1));
    auto IDX2 = dyn_cast<Instruction>(GP2->getOperand(1));

    if (IDX1->getOpcode() == Instruction::ZExt || 
        IDX1->getOpcode() == Instruction::SExt) {
        IDX1 = dyn_cast<Instruction>(IDX1->getOperand(0));
    }
    if (IDX2->getOpcode() == Instruction::ZExt || 
        IDX2->getOpcode() == Instruction::SExt) {
        IDX2 = dyn_cast<Instruction>(IDX2->getOperand(0));
    }
    // Operands of addition
    Value *ADD1;
    Value *ADD2;
    ConstantInt *C1;
    ConstantInt *C2;
    if (!match(IDX1, m_Add(m_ConstantInt(C1), m_Value(ADD1))) &&
        !match(IDX1, m_Add(m_Value(ADD1), m_ConstantInt(C1)))) {
         return false;
    }

    if (!match(IDX2, m_Add(m_ConstantInt(C2), m_Value(ADD2))) &&
        !match(IDX2, m_Add(m_Value(ADD2), m_ConstantInt(C2)))) {
         return false;
    }

    if (!isIdentical(ADD1, ADD2)) {
        return false;
    }

    *difference = C2->getZExtValue() - C1->getZExtValue();
    return true;
}


void AbbrMemPass::processBasicBlock(BasicBlock *BB) {
    auto callList = getInst(BB, Instruction::Load);
    getSequence(callList);
}


PreservedAnalyses AbbrMemPass::run(Function& F, FunctionAnalysisManager& FAM) {
    for (auto &BB : F) {
        processBasicBlock(&BB);
    }
    return PreservedAnalyses::all();
}
}