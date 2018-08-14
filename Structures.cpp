#include "Structures.h"

map<string,SymbolEntry> symbols;
map<string, DataEntry> sections;

set<string> ConditionNames = {
    "eq", "ne", "gt", "al"
};

set<string> InstructionNames = {
    "add", "sub", "mul", "div", "cmp", "and", "or", "not", "test", "push", "pop",
    "call", "iret", "mov", "shl", "shr"
};

set<string>  DirectiveNames = {
    ".global", ".char", ".word", ".long", ".align", ".skip", ".end"
};

set<string> SectionNames = {
    ".undefined", ".text", ".data", ".rodata", ".bss"
};

set<string> RegisterNames = {
    "r0", "r1", "r2", "r3", "r4", "r5"
};

vector<string> errors = {};

Error::Error(string msg){
    this->message = msg;
}

string Error::getMessage() {
    return this->message;
}
