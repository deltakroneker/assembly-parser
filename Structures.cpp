#include "Structures.h"

set<string> ConditionNames = {
    "eq", "ne", "gt", "al"
};

set<string> OperationNames = {
    "add", "sub", "mul", "div", "cmp", "and", "or", "not", "text", "push", "pop",
    "call", "iret", "mov", "shl", "shr", "tst", "ret", "jmp"
};

set<string>  DirectiveNames = {
    ".global", ".char", ".word", ".long", ".align", ".skip"
};

set<string> SectionNames = {
    ".text", ".data", ".rodata", ".bss"
};

set<string> RegisterNames = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

vector<string> errors = {};

Error::Error(string msg){
    this->message = msg;
}

string Error::getMessage() {
    return this->message;
}
