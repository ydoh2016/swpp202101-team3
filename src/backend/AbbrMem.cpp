#include "../core/Team3Passes.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"

#include <vector>
#include <algorithm>

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {
int AbbrMemPass::MAXSEQ = 8;

void AbbrMemPass::getInst(BasicBlock *BB, unsigned opcode, vector<Instruction*> *instList) {
    for (auto &I : *BB) {
        if (I.getOpcode() == opcode) {
            instList->push_back(&I);
        }
    }
}

void AbbrMemPass::getSequences(const vector<Instruction*> &instList, vector<vector<Instruction*>> *sequences) {
    vector<Instruction*> sequence; // A sequence consists of elements in a memory in a row
    int sz = instList.size();
    vector<bool> isRemaining(sz, true); // To prevent repetitive sequences

    // Combine each sequence into a vector and push into the sequences vector
    for (int i = 0; i < sz; ++i) {
        if (!isRemaining[i]) {
            continue;
        }

        Instruction *I1 = instList[i];
        sequence = vector<Instruction*>(MAXSEQ);
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
            sequences->push_back(sequence);
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
    auto IDX1 = GP1->getOperand(1);
    auto IDX2 = GP2->getOperand(1);

    if (isa<ConstantInt>(IDX1) && isa<ConstantInt>(IDX2)) {
        *difference = dyn_cast<ConstantInt>(IDX2)->getZExtValue() - 
        dyn_cast<ConstantInt>(IDX1)->getZExtValue();
        return true;
    }

    auto INST1 = dyn_cast<Instruction>(IDX1);
    auto INST2 = dyn_cast<Instruction>(IDX2);
    if (!INST1 || !INST2) {
        return false;
    }

    if (INST1->getOpcode() == Instruction::ZExt || 
        INST1->getOpcode() == Instruction::SExt) {
        INST1 = dyn_cast<Instruction>(INST1->getOperand(0));
    }
    if (INST2->getOpcode() == Instruction::ZExt || 
        INST2->getOpcode() == Instruction::SExt) {
        INST2 = dyn_cast<Instruction>(INST2->getOperand(0));
    }
    // Operands of addition
    Value *ADD1;
    Value *ADD2;
    ConstantInt *C1;
    ConstantInt *C2;
    if (!match(INST1, m_Add(m_ConstantInt(C1), m_Value(ADD1))) &&
        !match(INST1, m_Add(m_Value(ADD1), m_ConstantInt(C1)))) {
         return false;
    }

    if (!match(INST2, m_Add(m_ConstantInt(C2), m_Value(ADD2))) &&
        !match(INST2, m_Add(m_Value(ADD2), m_ConstantInt(C2)))) {
         return false;
    }

    if (!isIdentical(ADD1, ADD2)) {
        return false;
    }

    *difference = C2->getZExtValue() - C1->getZExtValue();
    return true;
}

int AbbrMemPass::getMask(const vector<Instruction*> &sequence) {
    int mask = 0;
    int sz = sequence.size();
    for (int i = 0; i < sz; ++i) {
        auto seq = sequence[i];
        if (seq != nullptr) {
            mask |= (1 << i);
        }
    }
    return mask;
}

void AbbrMemPass::insertFunctionCall(const vector<vector<Instruction*>> &sequences, BasicBlock *BB) {
    auto &bbContext = BB->getContext();
    auto int64PtrType = Type::getInt64PtrTy(bbContext);
    auto int64Type = Type::getInt64Ty(bbContext);
    auto int64VectorType = VectorType::get(int64Type, 8, false);

    vector<Type*> vload8Args = {int64PtrType, int64Type}; // Pointer from which we retrieve an element / mask
    auto vload8ty = FunctionType::get(int64VectorType, vload8Args, false); 
    auto vload8 = Function::Create(vload8ty, Function::ExternalLinkage, "vload8");

    vector<Type*> extract8Args = {int64VectorType, int64Type};
    auto extract8ty = FunctionType::get(int64Type, extract8Args, false); 
    auto extract8 = Function::Create(extract8ty, Function::ExternalLinkage, "extract_element8");

    for (auto &sequence : sequences) {
        Instruction *start = sequence[0];
        // Implement the mask
        auto VMask = ConstantInt::get(Type::getInt64Ty(BB->getContext()), getMask(sequence));
        vector<Value*> vload8Args = {start->getOperand(0), VMask};
        Instruction *vload8Call = CallInst::Create(vload8ty, vload8, vload8Args, "", start);
        for (int i = 0; i < MAXSEQ; ++i) {
            if (sequence[i] != nullptr) {
                auto VIdx = ConstantInt::get(Type::getInt64Ty(BB->getContext()), i);
                vector<Value*> extract8Args = {vload8Call, VIdx};
                auto extract8Call = CallInst::Create(extract8ty, extract8, extract8Args, "", start);
                //ReplaceInstWithInst(sequence[i], extract8Call);
                sequence[i]->replaceAllUsesWith(extract8Call);
            }
        }
    }
}


void AbbrMemPass::processBasicBlock(BasicBlock *BB) {
    vector<Instruction*> loads;
    vector<vector<Instruction*>> loadSequences;
    getInst(BB, Instruction::Load, &loads);
    getSequences(loads, &loadSequences);
    insertFunctionCall(loadSequences, BB);
}

// Declare vector memory access functions
void AbbrMemPass::addDeclarations(Module *M) {
    auto &mContext = M->getContext();
    auto int64PtrType = Type::getInt64PtrTy(mContext);
    auto int64Type = Type::getInt64Ty(mContext);
    auto int64VectorType = VectorType::get(int64Type, 8, false);

    vector<Type*> vload8Args = {int64PtrType, int64Type}; // Pointer from which we retrieve an element / mask
    auto vload8ty = FunctionType::get(int64VectorType, vload8Args, false); 
    auto vload8 = Function::Create(vload8ty, Function::ExternalLinkage, "vload8", M);

    vector<Type*> extract8Args = {int64VectorType, int64Type};
    auto extract8ty = FunctionType::get(int64Type, extract8Args, false); 
    auto extract8 = Function::Create(extract8ty, Function::ExternalLinkage, "extract_element8", M);
}


PreservedAnalyses AbbrMemPass::run(Module& M, ModuleAnalysisManager& MAM) {
    addDeclarations(&M);

    for (auto &F : M) {
        for (auto &BB : F) {
            processBasicBlock(&BB);
        }
    }
    
    return PreservedAnalyses::none();
}
}