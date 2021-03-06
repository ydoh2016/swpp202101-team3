#include "Backend.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace backend {

//---------------------------------------------------------------
//class TargetMachine
//---------------------------------------------------------------

Register* TargetMachine::reg(unsigned index) {
    return regfile[index];
}
Register* TargetMachine::arg(unsigned index) {
    return argfile[index];
}
Register* TargetMachine::sp() {
    return spreg;
}
Register* TargetMachine::fp() {
    return fpreg;
}
Register* TargetMachine::gvp() {
    return gvpreg;
}

Register* TargetMachine::sgvp() {
    return sgvpreg;
}
Register* TargetMachine::fakeReg() {
    return fakereg;
}
unsigned TargetMachine::regNo(Symbol* symbol) {
    for(int i = 0; i < USER_REGISTER_NUM; i++) {
        if(symbol == regfile[i]) return i;
    }
    return -1;
}
unsigned TargetMachine::argNo(Symbol* symbol) {
    for(int i = 0; i < 16; i++) {
        if(symbol == argfile[i]) return i;
    }
    return -1;
}
bool TargetMachine::valid(Symbol* symbol) {
    if(symbol) {
        if(symbol->castToRegister()) {
            for(int i = 0; i < 16; i++) {
            if(argfile[i] == symbol) return true;
            }
            for(int i = 0; i < USER_REGISTER_NUM; i++) {
            if(regfile[i] == symbol) return true;
            }
            if(spreg == symbol) return true;
            if(gvpreg == symbol) return true;
            if(sgvpreg == symbol) return true;
            if(fakereg == symbol) return true;
        }
        //Every implicit memory addresses are expressed as a base register(sp, gvp) and offset.
        else if(symbol->castToMemory()) {
            return valid((symbol)->castToMemory()->getBase());
        }
    }
    return false;
}

TargetMachine::TargetMachine() {
    for(int i = 0; i < 16; i++) {
        argfile[i] = new Register("arg"+to_string(i+1));
    }
    for(int i = 0; i < USER_REGISTER_NUM; i++) {
        regfile[i] = new Register("r"+to_string(i+1));
    }
    fpreg = new Register("r32");
    spreg = new Register("sp");
    gvpreg = new Register("gvp");
    sgvpreg = new Register("sgvp");
    fakereg = new Register("fake");
}

//---------------------------------------------------------------
//class Symbol & its families
//---------------------------------------------------------------

//Class that represents a hardware symbol corresponding to IR symbol
string Symbol::getName() {
    return name;
}
Memory::Memory(Register* base, int64_t offset) : base(base), offset(offset) {
    this->name = base->getName() + to_string(offset);
}
Register* Memory::getBase() {
    return base;
}
int64_t Memory::getOffset() {
    return offset;
}

}
