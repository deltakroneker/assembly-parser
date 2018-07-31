#ifndef STRUCTURES_H
#define	STRUCTURES_H
#include <iostream>
#include <set>
#include <map>
#include <vector>
using namespace std;

enum Visibility {
    LOCAL, GLOBAL
};

enum SymbolType {
    OPERATION, DIRECTIVE, SECTION, EMPTY, UNDEFINED
};

extern set<string> ConditionNames;
extern set<string> OperationNames;
extern set<string> DirectiveNames;
extern set<string> SectionNames;
extern set<string> RegisterNames;

struct Instruction {
    string label;
    string mnemonic;
    vector<string> operands;
};

struct Directive {
    string label;
    string name;
    vector<string> operands;
};

struct Section {
    string label;
    string name;
};

struct SymbolEntry {
    
    SymbolType type;
    Instruction operation;
    Directive directive;
    Section section;
    
    string name;
    int number;
    Visibility visibility;
    
    // For section & directive type
    unsigned size;
    unsigned address;

    // For operation type
    unsigned value;

};

extern map<string, SymbolEntry> symbols;
extern vector<string> errors;

class Error {
    string message;
public:
    Error(string msg);
    string getMessage();
};


#endif

