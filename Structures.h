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
    SECTION, DIRECTIVE, INSTRUCTION, EMPTY, UNDEFINED
};

enum ConditionType {
    EQ, NE, GT, AL
};

extern set<string> ConditionNames;
extern set<string> InstructionNames;
extern set<string> DirectiveNames;
extern set<string> SectionNames;
extern set<string> RegisterNames;

struct SymbolEntry {
    unsigned id;
    string type;
    string section;
    unsigned size;
    unsigned value;
    char scope;
};

struct DataEntry {
    vector<unsigned char> data;
};

extern map<string, SymbolEntry> symbols;
extern map<string, DataEntry> sections;

extern vector<string> errors;

class Error {
    string message;
public:
    Error(string msg);
    string getMessage();
};

class End {};

#endif

