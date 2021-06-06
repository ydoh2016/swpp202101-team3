#include "Backend.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace {
    // Return sizeof(T) in bytes.
    unsigned getAccessSize(Type *T) {
        if (isa<PointerType>(T))
            return 8;
        else if (isa<IntegerType>(T)) {
            return T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8);
        } else if (isa<ArrayType>(T)) {
            return getAccessSize(T->getArrayElementType()) * T->getArrayNumElements();
        }
        assert(false && "Unsupported access size type!");
    }
}

namespace backend {

string AssemblyEmitter::name(Value* v) {
    if(!v || isa<ConstantPointerNull>(v) || v->getType()->isVoidTy()) {
        return "0";
    }
    if(isa<ConstantInt>(v)) {
        //return the value itself.
        return to_string(dyn_cast<ConstantInt>(v)->getZExtValue());
    }
    return SM->get(v)->getName();
}

//static functions for emitting common formats.
string AssemblyEmitter::emitInst(vector<string> printlist) {
    string str = "  ";
    for(string s : printlist) {
        str += s + " ";
    }
    str += "\n";
    return str;
}
string AssemblyEmitter::emitBinary(Instruction* v, string opcode, string op1, string op2) {
    return emitInst({name(v), "=", opcode, op1, op2, stringBandWidth(v)});
}
string AssemblyEmitter::emitCopy(Instruction* v, Value* op) {
    Memory* mem = SM->get(op)? SM->get(op)->castToMemory() : NULL;
    if(mem) {
        if(mem->getBase() == TM->gvp()) {
            return emitBinary(v, "add", "204800", to_string(mem->getOffset()));    
        }
        return emitBinary(v, "add", mem->getBase()->getName(), to_string(mem->getOffset()));
    }
    return emitBinary(v, "mul", name(op), "1");
}

string AssemblyEmitter::stringBandWidth(Value* v) {
    if(isa<Function>(v) || isa<BasicBlock>(v)) {
        assert(false && "v should be a digit-typed value");
    }
    return to_string(getBitWidth(v->getType()));
}

AssemblyEmitter::AssemblyEmitter(raw_ostream *fout, TargetMachine& TM, SymbolMap& SM, map<Function*, SpInfo>& spOffset, set<string>& mallocLikes, map<Instruction*, Instruction*>& refOptiMemAccMap) :
            fout(fout), TM(&TM), SM(&SM), spOffset(spOffset), mallocLikeFunc(mallocLikes), optiMemAccMap(refOptiMemAccMap) {
    //base assembly code for heap to stack
    //if there's not enough space, it will do malloc
    *fout << "start _Alloca 2:\n"
    "._defaultBB0:\n"
    "  r1 = mul arg1 arg2 64\n"
    "  r2 = sub sp r1 64\n"
    "  r3 = icmp slt r2 10000 64\n"
    "  br r3 ._malloc ._alloca\n"
    "._malloc:\n"
    "  r2 = malloc r1\n"
    "  ret r2\n"
    "._alloca:\n"
    "  ret r2\n"
    "end _Alloca\n"
    "\n"
    "start _SpCal 1:\n"
    "  ._defaultBB0:\n"
    "  r1 = icmp ugt arg1 102400 64\n"
    "  br r1 ._malloc ._alloca\n"
    "._malloc:\n"
    "  ret sp\n"
    "._alloca:\n"
    "  ret arg1\n"
    "end _SpCal\n"
    "\n"
    "start _Free 1:\n"
    "  ._defaultBB0:\n"
    "  r1 = icmp ugt arg1 102400 64\n"
    "  br r1 ._malloc ._alloca\n"
    "._malloc:\n"
    "  free arg1\n"
    "  ret\n"
    "._alloca:\n"
    "  ret\n"
    "end _Free\n"
    "\n";
}

void AssemblyEmitter::visitFunction(Function& F) {
    //print the starting code.
    //finishing code will be printed outside the AssemblyEmitter.
    countOfLoad = 0;
    countOfStore = 0;
    *fout << "start " << name(&F) << " " << F.arg_size() << ":\n";
}
void AssemblyEmitter::visitBasicBlock(BasicBlock& BB) {
    *fout << "." << name(&BB) << ":\n";

    //If entry block, modify SP.
    if(&(BB.getParent()->getEntryBlock()) == &BB) {
        //if main, import GV within.
        //this code should happen only if GV array was in the initial program.
        //GV values are all lowered into alloca + calls
        if(BB.getParent()->getName() == "main" && BB.getModule()->getGlobalList().size()!=0) {
            *fout << "  ; Init global variables\n";
            for(auto& gv : BB.getModule()->globals()) {
                //temporarily stores the GV pointer.
                unsigned size = (getAccessSize(gv.getValueType()) + 7) / 8 * 8;
                *fout << emitInst({"r1 = malloc", to_string(size)});
                if(gv.hasInitializer() && !gv.getInitializer()->isZeroValue()) {
                    *fout << emitInst({"store", to_string(getAccessSize(gv.getValueType())), name(gv.getInitializer()), "r1 0"});
                }
            }
        }
        if(spOffset[BB.getParent()].touched) {
            *fout << "  ; Init stack pointer\n";
            if(spOffset[BB.getParent()].acc > 0) {
                *fout << emitInst({"sp = sub sp",to_string(spOffset[BB.getParent()].acc),"64"});
            }
            *fout << emitInst({"r32 = mul sp", "1","64"});
        }
    }
}

//Compare insts.
void AssemblyEmitter::visitICmpInst(ICmpInst& I) {
    *fout << emitInst({name(&I), "= icmp", I.getPredicateName(I.getPredicate()).str(), name(I.getOperand(0)), name(I.getOperand(1)), stringBandWidth(I.getOperand(0))});
}

//Alloca inst.
void AssemblyEmitter::visitAllocaInst(AllocaInst& I) {
    
    Value* ptr = I.getArraySize();
    string size = to_string(getAccessSize(I.getAllocatedType()));
    Symbol* symbol = SM->get(ptr);
    if(I.isStaticAlloca()) {
        
    }
    else if(symbol) {
        if(Register* reg = symbol->castToRegister()) {
            
            *fout << emitInst({name(&I), "= call _Alloca", size, reg->getName()});
            //remove redundant and expensive _SpCal call when ptr returns directly.
            if(I.hasOneUse()) {
                auto ri = dyn_cast<ReturnInst>(*(I.user_begin()));
                if(ri)
                    return;
            }
            *fout << emitInst({"sp", "= call _SpCal", name(&I)});
        }
    }
}

//Memory Access insts.
void AssemblyEmitter::visitLoadInst(LoadInst& I) {
    bool opti = false;
    auto it = optiMemAccMap.find(&I);
    if(it != optiMemAccMap.end()) {
        opti = true;
    }
    Value* ptr = I.getPointerOperand();
    //bytes to load
    string size = to_string(getAccessSize(dyn_cast<PointerType>(ptr->getType())->getElementType()));
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(Memory* mem = symbol->castToMemory()) {
        if(mem->getBase() == TM->fp()) {
            *fout << emitInst({name(&I), "= load", size, mem->getBase()->getName(), to_string(mem->getOffset())});
        }
        else if(mem->getBase() == TM->gvp()) {
            *fout << emitInst({name(&I), "= load", size, "204800", to_string(mem->getOffset())});
        }
        else assert(false && "base of memory pointers should be sp or gvp");
    }
    //else a pointer stored in register,
    else if(Register* reg = symbol->castToRegister()) {
        if(opti) {
            string tmpReg = "r" + to_string((USER_REGISTER_NUM - remainRegister) + 1);
            auto allocInst = dyn_cast<AllocaInst>(it->second);
            Symbol* symbol = SM->get(allocInst);
            if(Memory* mem = symbol->castToMemory()) {
                auto elemSize = (getAccessSize(allocInst->getAllocatedType()->getArrayElementType()));
                auto elemCount = allocInst->getAllocatedType()->getArrayNumElements();
                bool coverAll = (remainRegister-1) >= elemCount;
                *fout << "  " << tmpReg << "= sub " << reg->getName() << " " << mem->getBase()->getName() << " 64\n";
                *fout << "  " << tmpReg << "= udiv " << tmpReg << " " << elemSize << " 64\n";
                *fout << "  " << "switch " << tmpReg;
                if(coverAll) {
                    for(auto i = 0; i < elemCount; ++i) {
                        *fout << " " << i << " .load" << countOfLoad << "." << i;
                    }
                    *fout << " .normalLoad" << countOfLoad << "\n";
                    for(auto i = 0; i < elemCount; ++i) {
                        *fout << ".load" << countOfLoad << "." << i << ":\n";
                        *fout << "  " << name(&I) << " = mul 1 " << "r" << (USER_REGISTER_NUM - remainRegister) + 2 + i << " 64\n";
                        *fout << "  " << "br .after.load" << countOfLoad << "\n"; 
                    }
                    *fout << ".normalLoad" << countOfLoad << ":\n";
                    *fout<<"  " << name(&I) << " = load " << size << " " << reg->getName() << " 0\n";
                    *fout<<"  " << "br .after.load" << countOfLoad << "\n"; 
                    *fout<<".after.load" << countOfLoad << ":\n"; 
                }
                else {
                    for(auto i = 0; i < remainRegister-1; ++i) {
                        *fout << " " << i << " .load" << countOfLoad << "." << i;
                    }
                    *fout << " .normalLoad" << countOfLoad << "\n";
                    for(auto i = 0; i < remainRegister-1; ++i) {
                        *fout<<"  " << ".load" << countOfLoad << "." << i << ":\n";
                        *fout<<"  " << name(&I) << " = mul 1 " << "r" << (USER_REGISTER_NUM - remainRegister) + 2 + i << " 64\n";
                        *fout<<"  " << "br .after.load" << countOfLoad << "\n"; 
                    }
                    *fout << ".normalLoad" << countOfLoad << ":\n";
                    *fout<<"  " << name(&I) << " = load " << size << " " << reg->getName() << " 0\n";
                    *fout<<"  " << "br .after.load" << countOfLoad << "\n"; 
                    *fout<<".after.load" << countOfLoad << ":\n"; 
                }
                countOfLoad += 1;
            }
            else {
                *fout << emitInst({name(&I), "= load", size, reg->getName(), "0"});    
            }
        }
        else {
            *fout << emitInst({name(&I), "= load", size, reg->getName(), "0"});
        }
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}
void AssemblyEmitter::visitStoreInst(StoreInst& I) {
    bool opti = false;
    auto it = optiMemAccMap.find(&I);
    if(it != optiMemAccMap.end()) {
        opti = true;
    }
    Value* ptr = I.getPointerOperand();
    //bytes to load
    string size = to_string(getAccessSize(dyn_cast<PointerType>(ptr->getType())->getElementType()));
    Value* val = I.getValueOperand();
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(Memory* mem = symbol->castToMemory()) {
        if(mem->getBase() == TM->fp()) {
            *fout << emitInst({"store", size, name(val), mem->getBase()->getName(), to_string(mem->getOffset())});
        }
        else if(mem->getBase() == TM->gvp()) {
            *fout << emitInst({"store", size, name(val), "204800", to_string(mem->getOffset())});
        }
        else assert(false && "base of memory pointers should be sp or gvp");
    }
    //else a pointer stored in register,
    else if(Register* reg = symbol->castToRegister()) {
        if(opti) {
            string tmpReg = "r" + to_string((USER_REGISTER_NUM - remainRegister) + 1);
            auto allocInst = dyn_cast<AllocaInst>(it->second);
            Symbol* symbol = SM->get(allocInst);
            if(Memory* mem = symbol->castToMemory()) {
                auto elemSize = (getAccessSize(allocInst->getAllocatedType()->getArrayElementType()));
                auto elemCount = allocInst->getAllocatedType()->getArrayNumElements();
                bool coverAll = (remainRegister-1) >= elemCount;
                *fout << "  " << tmpReg << "= sub " << reg->getName() << " " << mem->getBase()->getName() << " 64\n";
                *fout << "  " << tmpReg << "= udiv " << tmpReg << " " << elemSize << " 64\n";
                *fout << "  " << "switch " << tmpReg;
                if(coverAll) {
                    for(auto i = 0; i < elemCount; ++i) {
                        *fout << " " << i << " .store" << countOfStore << "." << i;
                    }
                    *fout << " .normalStore" << countOfStore << "\n";
                    for(auto i = 0; i < elemCount; ++i) {
                        *fout << ".store" << countOfStore << "." << i << ":\n";
                        *fout << "  " << "r" << (USER_REGISTER_NUM - remainRegister) + 2 + i << " = mul 1 " << name(val) << " 64\n";
                        *fout << "  " << "br .after.store" << countOfStore << "\n"; 
                    }
                    *fout << ".normalStore" << countOfStore << ":\n";
                    *fout << emitInst({"store", size, name(val),reg->getName(), "0"});
                    *fout<<"  " << "br .after.store" << countOfStore << "\n"; 
                    *fout<<".after.store" << countOfStore << ":\n"; 
                }
                else {
                    for(auto i = 0; i < remainRegister-1; ++i) {
                        *fout << " " << i << " .store" << countOfStore << "." << i;
                    }
                    *fout << " .normalStore" << countOfStore << "\n";
                    for(auto i = 0; i < remainRegister-1; ++i) {
                        *fout<< ".store" << countOfStore << "." << i << ":\n";
                        *fout << "  " << "r" << (USER_REGISTER_NUM - remainRegister) + 2 + i << " = mul 1 " << name(val) << " 64\n";
                        *fout<<"  " << "br .after.store" << countOfStore << "\n"; 
                    }
                    *fout << ".normalStore" << countOfStore << ":\n";
                    *fout << emitInst({"store", size, name(val),reg->getName(), "0"});
                    *fout<<"  " << "br .after.store" << countOfStore << "\n"; 
                    *fout<<".after.store" << countOfStore << ":\n"; 
                }
                countOfStore += 1;
            }
            else {
                *fout << emitInst({"store", size, name(val),reg->getName(), "0"});
            }
        }
        else {
            *fout << emitInst({"store", size, name(val),reg->getName(), "0"});
        }
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}

//PHI Node inst.
void AssemblyEmitter::visitPHINode(PHINode& I) {
    //Do nothing.
}

//Reformatting(no value changes) insts.
void AssemblyEmitter::visitTruncInst(TruncInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitZExtInst(ZExtInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitSExtInst(SExtInst& I) {
    unsigned beforeBits = getBitWidth(I.getOperand(0)->getType());
    unsigned afterBits = getBitWidth(I.getType());
    assert(afterBits > beforeBits && "SExt must increase the bandwidth");
    *fout << emitBinary(&I, "mul", name(I.getOperand(0)), to_string(1llu<<(afterBits-beforeBits)));
    *fout << emitBinary(&I, "sdiv", name(&I), to_string(1llu<<(afterBits-beforeBits)));
}
void AssemblyEmitter::visitPtrToIntInst(PtrToIntInst& I) {
    Value* ptr = I.getPointerOperand();
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(symbol) {
        if(Memory* mem = symbol->castToMemory()) {
            if(mem->getBase() == TM->fp()) {
                *fout << emitBinary(&I, "add", mem->getBase()->getName(), to_string(mem->getOffset()));
            }
            else if(mem->getBase() == TM->gvp()) {
                *fout << emitBinary(&I, "add", "204800", to_string(mem->getOffset()));
            }
            else assert(false && "base of memory pointers should be sp or gvp");
        }
        //else a pointer stored in register,
        else if(Register* reg = symbol->castToRegister()) {
            //if from and to values are stored in a different source, copy.
            if(SM->get(&I) != SM->get(I.getOperand(0))) {
                *fout << emitCopy(&I, I.getOperand(0));
            }
        }
        return;
    }
    //else ptr is null
    if(isa<ConstantPointerNull>(ptr)) {
        *fout << emitBinary(&I, "mul", "0", "0");
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}
void AssemblyEmitter::visitIntToPtrInst(IntToPtrInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitBitCastInst(BitCastInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}

//Select inst.
void AssemblyEmitter::visitSelectInst(SelectInst& I) {
    *fout << emitInst({name(&I), "= select", name(I.getCondition()), name(I.getTrueValue()), name(I.getFalseValue())});
}

void AssemblyEmitter::visitCallInst(CallInst& I) {
    //Process malloc()&free() from other plain call insts.
    Function* F = I.getCalledFunction();
    string Fname = F->getName().str();
    
    //Collect all arguments
    vector<string> args;
    for(Use& arg : I.args()) {
        args.push_back(name(arg.get()));
    }
    //Find all memory allocation call, apply special stack pointer update call
    if(mallocLikeFunc.find(Fname) != mallocLikeFunc.end()) {
        vector<string> printlist = {name(&I), "= call", Fname};
        printlist.insert(printlist.end(), args.begin(), args.end());
        *fout << emitInst(printlist);
        *fout << emitInst({"sp", "= call _SpCal", name(&I)});
    }
    else if(Fname == "malloc") {
        assert(args.size()==1 && "argument of malloc() should be 1");
        *fout << emitInst({name(&I), "= malloc", name(I.getArgOperand(0))});
    }
    else if(Fname == "free") {
        assert(args.size()==1 && "argument of free() should be 1");
        //to optimize free operation
        //If it is sure that ptr is on the heap, then call free not _Free
        //because _Free is expensive than free by about 6 point.
        auto ii = dyn_cast<Instruction>(I.getArgOperand(0));
        BitCastInst* bi = dyn_cast<BitCastInst>(ii);
        while(bi) {
            ii = dyn_cast<Instruction>(bi->getOperand(0));
            bi = dyn_cast<BitCastInst>(bi->getOperand(0));
        }
        auto ci = dyn_cast<CallInst>(ii);
        if(ci) {
            auto calledFunc = ci->getCalledFunction();
            if(mallocLikeFunc.find(calledFunc->getName().str()) == mallocLikeFunc.end()) {
                *fout << emitInst({"free", name(I.getArgOperand(0))});        
                return;
            }
        }
        *fout << emitInst({"call", "_Free", name(I.getArgOperand(0))});
    }
    else if(UnfoldVectorInstPass::VLOADS.find(Fname) != UnfoldVectorInstPass::VLOADS.end()) {
        vector<string> asmb;
        int n = atoi(Fname.substr(Fname.size() - 1, 1).c_str());
        for (int i=0; i<n; i++) asmb.push_back("_");

        auto it = I.getIterator();
        for (it++; ;) {
            CallInst *NI = dyn_cast<CallInst>(&*it++);
            if (NI == NULL) break;

            string niFn = NI->getCalledFunction()->getName().str();
            if (niFn != "extract_element2" && niFn != "extract_element4" && niFn != "extract_element8") break;

            ConstantInt *C = dyn_cast<ConstantInt>(NI->getOperand(1));
            assert(C != NULL && "extract_element should retrieve a constant argument as a dim");

            int pos = C->getZExtValue();
            assert(0 <= pos && pos < asmb.size());

            asmb[pos] = name(NI);
        }

        asmb.push_back("= vload");
        asmb.push_back(Fname.substr(Fname.size() - 1, 1)); // n
        asmb.push_back(name(I.getOperand(0))); // ptr
        asmb.push_back("0"); // offset = 0

        *fout << emitInst(asmb);
    }
    else if(UnfoldVectorInstPass::VSTORES.find(Fname) != UnfoldVectorInstPass::VSTORES.end()) {
        vector<string> asmb;
        int n = atoi(Fname.substr(Fname.size() - 1, 1).c_str());

        asmb.push_back("vstore");
        asmb.push_back(Fname.substr(Fname.size() - 1, 1)); // n
        for (int i=0; i<n; i++) asmb.push_back("_");

        auto *maskVal = dyn_cast<ConstantInt>(I.getOperand(n + 1));
        assert(maskVal != NULL && "mask should be a constant integer");

        int mask = maskVal->getZExtValue();
        assert(0 <= mask && mask < (1 << n) && "invalid mask number");

        for (int i=0; i<n; i++) {
            if (!(mask & (1 << i))) continue;

            Value *V = I.getOperand(i);
            asmb[2+i] = name(V);
        }

        asmb.push_back(name(I.getOperand(n))); // ptr
        asmb.push_back("0"); // offset = 0

        *fout << emitInst(asmb);
    }
    else if(UnfoldVectorInstPass::EXTRACT_ELEMENTS.find(Fname) != UnfoldVectorInstPass::EXTRACT_ELEMENTS.end()) {
        // do nothing
    }
	else if(F->getReturnType()->isVoidTy()) {
		vector<string> printlist = {"call", Fname};
		printlist.insert(printlist.end(), args.begin(), args.end());
		*fout << emitInst(printlist);
	}
    //ordinary function calls.
    else {
        vector<string> printlist = {name(&I), "= call", Fname};
        printlist.insert(printlist.end(), args.begin(), args.end());
        *fout << emitInst(printlist);
    }
}

//Terminator insts.
void AssemblyEmitter::visitReturnInst(ReturnInst& I) {
    //increase sp(which was decreased in the beginning of the function.)
    Function* F = I.getFunction();
    // From the calling convention - which says after the call returns, r1~r32, sp registers are automatically restored
    // We don't need following calculations.
    // if(spOffset[F].touched)
    //     *fout << emitInst({"sp = mul r32",to_string(1),"64"});
    // if(spOffset[F].acc > 0) {
    //     *fout << emitInst({"sp = add sp",to_string(spOffset[F].acc),"64"});
    // }
    *fout << emitInst({"ret", name(I.getReturnValue())});
}
void AssemblyEmitter::visitBranchInst(BranchInst& I) {
    if(I.isConditional()) {
        assert(I.getNumSuccessors() == 2 && "conditional branches must have 2 successors");
        *fout << emitInst({"br", name(I.getCondition()), "." + name(I.getSuccessor(0)), "." + name(I.getSuccessor(1))});
    }
    else {
        assert(I.getNumSuccessors() == 1 && "unconditional branches must have 1 successor");
        *fout << emitInst({"br", "." + name(I.getSuccessor(0))});
    }
}
void AssemblyEmitter::visitSwitchInst(SwitchInst& I) {
    string asmb("switch " + name(I.getCondition()));
    for(auto& c : I.cases()) {
        if(c.getCaseIndex() == I.case_default()->getCaseIndex()) continue;
        asmb.append(" " + name(c.getCaseValue()) + " ." + name(c.getCaseSuccessor()));
    }
    asmb.append(" ." + name(I.case_default()->getCaseSuccessor()));
    *fout << asmb << "\n";
}
void AssemblyEmitter::visitBinaryOperator(BinaryOperator& I) {
    string opcode = "";
    switch(I.getOpcode()) {
    case Instruction::UDiv: opcode = "udiv"; break;
    case Instruction::SDiv: opcode = "sdiv"; break;
    case Instruction::URem: opcode = "urem"; break;
    case Instruction::SRem: opcode = "srem"; break;
    case Instruction::Mul:  opcode = "mul"; break;
    case Instruction::Shl:  opcode = "shl"; break;
    case Instruction::AShr: opcode = "ashr"; break;
    case Instruction::LShr: opcode = "lshr"; break;
    case Instruction::And:  opcode = "and"; break;
    case Instruction::Or:   opcode = "or"; break;
    case Instruction::Xor:  opcode = "xor"; break;
    case Instruction::Add:  opcode = "add"; break;
    case Instruction::Sub:  opcode = "sub"; break;
    default: assert(false && "undefined binary operation");
    }

    *fout << emitBinary(&I, opcode, name(I.getOperand(0)), name(I.getOperand(1)));
}

}
