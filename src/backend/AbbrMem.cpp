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
int AbbrMemPass::MAXSEQ = 8; // How many elements to deal with at once

// Get instructions with a certain operation
void AbbrMemPass::getInst(BasicBlock *BB, unsigned opcode, vector<Instruction*> *instList) {
    for (auto &I : *BB) {
        if (I.getOpcode() == opcode) {
            instList->push_back(&I);
        }
    }
}

// Get sequential memory access
void AbbrMemPass::getSequences(const vector<Instruction*> &instList, vector<vector<Instruction*>> *sequences) {
    vector<Instruction*> sequence; // A sequence consists of elements in a memory in a row
    int sz = instList.size();
    vector<bool> isRemaining(sz, true); // To prevent repetitive sequences
    // Combine each sequence into a vector then push into the sequences vector
    for (int i = 0; i < sz; ++i) {
        if (!isRemaining[i]) { // If the instruction has been accessed
            continue; // Skip this instruction
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
            // Are these two instructions in sequence?
            bool isSequence = inSameSequence(I1, I2, diff);
            // If they are in a row and their difference is less than MAXSEQ
            if (isSequence && *diff < MAXSEQ) {
                sequence[*diff] = I2;
                isRemaining[j] = false;
            } else {
                continue;
            }
        }
        // Determine as a sequence only if if has more than one instruction
        // in each vector
        if (std::count(sequence.begin(), sequence.end(), nullptr) <= MAXSEQ - 2) {
            sequences->push_back(sequence);
        }
    }
}

// Recursively check if two values are identical
bool AbbrMemPass::isIdentical(Value *V1, Value *V2) {
    // If both are constants, answer: are their values equal?
    if (isa<ConstantInt>(V1) && isa<ConstantInt>(V2)) {
        ConstantInt *C1 = dyn_cast<ConstantInt>(V1);
        ConstantInt *C2 = dyn_cast<ConstantInt>(V2);
        return C1->getZExtValue() == C2->getZExtValue();
    }
    
    // If both have the same name, they're equal at least in a same BB
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
        // Recursively check equivalence for each operand
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

// Check if the two values are in sequence. If so, get their gap.
bool AbbrMemPass::inSameSequence(Value *V1, Value *V2, int *difference) {
    // %2 = load i64, i64* %hptr1
    auto I1 = dyn_cast<Instruction>(V1);
    auto I2 = dyn_cast<Instruction>(V2);

    // %2 = load i64, [i64* %hptr1]
    // %1 = store i64 10, [i64* %hptr1]
    int pointerIdx = I1->getOpcode() == Instruction::Load ? 0 : 1;
    auto GP1 = dyn_cast<Instruction>(I1->getOperand(pointerIdx));
    auto GP2 = dyn_cast<Instruction>(I2->getOperand(pointerIdx));

    if (!GP1 || !GP2) {
        return false;
    }

    // %hptr1 = getelementptr inbounds i64, [i64* %hptr0], i64 1
    auto P1 = GP1->getOperand(0);
    auto P2 = GP2->getOperand(0);

    // Are the pointers same?
    if (P1 != P2) { 
        return false;
    }

    // %hptr1 = getelementptr inbounds i64, i64* %hptr0, [i64 1]
    auto IDX1 = GP1->getOperand(1);
    auto IDX2 = GP2->getOperand(1);

    // %hptr1 = getelementptr inbounds i64, i64* %hptr0, [i64 1]
    // %hptr2 = getelementptr inbounds i64, i64* %hptr0, [i64 2]
    if (isa<ConstantInt>(IDX1) && isa<ConstantInt>(IDX2)) {
        *difference = dyn_cast<ConstantInt>(IDX2)->getZExtValue() - 
        dyn_cast<ConstantInt>(IDX1)->getZExtValue();
        return true;
    }

    // %hptr1 = getelementptr inbounds i64, i64* %hptr0, [i64 %a]
    // %hptr2 = getelementptr inbounds i64, i64* %hptr0, [i64 %b]
    auto INST1 = dyn_cast<Instruction>(IDX1);
    auto INST2 = dyn_cast<Instruction>(IDX2);

    if (!INST1 || !INST2) {
        return false;
    }
    // If there are ZExt of SExt, assign their operands to INST1 to skip them
    if (INST1->getOpcode() == Instruction::ZExt || 
        INST1->getOpcode() == Instruction::SExt) {
        INST1 = dyn_cast<Instruction>(INST1->getOperand(0));
    }
    if (INST2->getOpcode() == Instruction::ZExt || 
        INST2->getOpcode() == Instruction::SExt) {
        INST2 = dyn_cast<Instruction>(INST2->getOperand(0));
    }
    // %a = add i64 %0, 1
    // %b = add i64 1, %1
    // Now the result is: %0 == %1 ? 
    Value *ADD1;
    Value *ADD2;
    ConstantInt *C1;
    ConstantInt *C2;
    // If the operation is not in a form of 'val + constant', return
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

// Get a mask for a sequence
// Only present elements are masked as 1
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
    auto int64VectorType = VectorType::get(int64Type, MAXSEQ, false);
    auto voidType = Type::getVoidTy(BB->getContext());

    bool isLoad = sequences[0][0]->getOpcode() == Instruction::Load;

    if (isLoad) {
        vector<Type*> vload8Args = {int64PtrType, int64Type};
        auto vload8ty = FunctionType::get(int64VectorType, vload8Args, false);
        // Creates a function with prototype '<8 x i64> (i64*, i64)' 
        auto vload8 = Function::Create(vload8ty, Function::ExternalLinkage, "vload8");

        vector<Type*> extract8Args = {int64VectorType, int64Type};
        auto extract8ty = FunctionType::get(int64Type, extract8Args, false); 
        auto extract8 = Function::Create(extract8ty, Function::ExternalLinkage, "extract_element8");

        for (auto &sequence : sequences) {
            Instruction *start = sequence[0];
            auto VMask = ConstantInt::get(Type::getInt64Ty(BB->getContext()), getMask(sequence));
            vector<Value*> vload8Args = {start->getOperand(0), VMask};
            Instruction *vload8Call = CallInst::Create(vload8ty, vload8, 
                                                       vload8Args, "", start);
            // Create extract8 calls following the vload8
            for (int i = 0; i < MAXSEQ; ++i) {
                if (sequence[i] != nullptr) {
                    auto VIdx = ConstantInt::get(Type::getInt64Ty(BB->getContext()), i);
                    vector<Value*> extract8Args = {vload8Call, VIdx};
                    auto extract8Call = CallInst::Create(extract8ty, extract8, 
                                                         extract8Args, "", start);
                    sequence[i]->replaceAllUsesWith(extract8Call);
                }
            }
        }
    } else {
        vector<Type*> vstore8Args = {int64Type, int64Type, int64Type, int64Type,
                                int64Type, int64Type, int64Type, int64Type,
                                int64PtrType, int64Type};
        auto vstore8ty = FunctionType::get(voidType, vstore8Args, false);
        auto vstore8 = Function::Create(vstore8ty, Function::ExternalLinkage, "vstore8");

        for (auto &sequence : sequences) {
            Instruction *start = sequence[0];
            auto VMask = ConstantInt::get(Type::getInt64Ty(BB->getContext()), getMask(sequence));
            vector<Value*> vstore8Args = {start->getOperand(0), VMask};
            //Instruction *vstore8Call = CallInst::Create(vstore8ty, vstore8, 
                                                        //vstore8Args, "", start);
        }
    }
}

// Try to optimize each block
void AbbrMemPass::processBasicBlock(BasicBlock *BB) {
    vector<Instruction*> loads;
    vector<vector<Instruction*>> loadSequences;
    getInst(BB, Instruction::Load, &loads);
    getSequences(loads, &loadSequences);
    insertFunctionCall(loadSequences, BB);

    vector<Instruction*> stores;
    vector<vector<Instruction*>> storeSequences;
    getInst(BB, Instruction::Store, &stores);
    getSequences(stores, &storeSequences);
    insertFunctionCall(storeSequences, BB);
}

// Declare vector memory access functions
void AbbrMemPass::addDeclarations(Module *M) {
    auto &mContext = M->getContext();
    auto int64PtrType = Type::getInt64PtrTy(mContext);
    auto int64Type = Type::getInt64Ty(mContext);
    auto int64VectorType = VectorType::get(int64Type, 8, false);
    auto voidType = Type::getVoidTy(mContext);

    vector<Type*> vload8Args = {int64PtrType, int64Type}; 
    auto vload8ty = FunctionType::get(int64VectorType, vload8Args, false); 
    auto vload8 = Function::Create(vload8ty, Function::ExternalLinkage, "vload8", M);

    vector<Type*> extract8Args = {int64VectorType, int64Type};
    auto extract8ty = FunctionType::get(int64Type, extract8Args, false); 
    auto extract8 = Function::Create(extract8ty, Function::ExternalLinkage, "extract_element8", M);

    vector<Type*> vstore8Args = {int64Type, int64Type, int64Type, int64Type,
                                 int64Type, int64Type, int64Type, int64Type,
                                 int64PtrType, int64Type};
    auto vstore8ty = FunctionType::get(voidType, vstore8Args, false); 
    auto vstore8 = Function::Create(vstore8ty, Function::ExternalLinkage, "vstore8", M);
}

PreservedAnalyses AbbrMemPass::run(Module &M, ModuleAnalysisManager &MAM) {
    addDeclarations(&M);
    for (auto &F : M) {
        for (auto &BB : F) {
            processBasicBlock(&BB);
        }
    }
    /*
    for (auto &F : M) {
        for (auto &BB : F) {
            for (auto &I : BB) {
                auto &mContext = M.getContext();
                auto int64PtrType = Type::getInt64PtrTy(mContext);
                auto int64Type = Type::getInt64Ty(mContext);
                auto int64VectorType = VectorType::get(int64Type, 8, false);
                auto voidType = Type::getVoidTy(mContext);

                vector<Type*> funcArgs = {int64Type}; 
                auto myFuncTy = FunctionType::get(voidType, funcArgs, false); 
                auto myFunc = Function::Create(myFuncTy, Function::ExternalLinkage, "myFunc", M);

                vector<Value*> callArgs = {ConstantInt::get(Type::getInt64Ty(M.getContext()), 5)};
                auto myFuncCall = CallInst::Create(myFuncTy, myFunc, callArgs, "", &I);
            }
        }
    }
    */

    return PreservedAnalyses::none();
}
}