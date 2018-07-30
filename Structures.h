#ifndef STRUCTURES_H
#define	STRUCTURES_H
#include <iostream>
using namespace std;

enum Section {
    text, data, rodata, bss
};

enum Visibility {
    local, global
};

enum SymbolType {
    section, operation
};

struct Symbol {
    SymbolType type;
    string name;
    int number;
    
    // For section type
    Section section;
    unsigned size;
    unsigned address;
    
    Symbol(string name, int number, Section section, unsigned size, unsigned address){
        this->type = section;
        this->name = name;
        this->number = number;
        this->section = section;
        this->size = size;
        this->address = address;
    }
    
    // For operation type
    unsigned value;
    
    Symbol(string name, int number, unsigned value){
        this->type = operation;
        this->name = name;
        this->number = number;
        this->value = value;
    }  
};

#endif

