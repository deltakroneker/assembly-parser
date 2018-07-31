#ifndef FIRSTPASS_H
#define	FIRSTPASS_H

#include <iostream>
#include "RegexParser.h"
#include "Structures.h"
using namespace std;

class FirstPass {
    
private:
    int number;
    unsigned locationCounter;
    string line;
    
    RegexParser* myRegex;
    void cleanUpLine(string& line);
    vector<string> splitStringBySpace(string line);
    void createSymbolEntry(SymbolEntry& entry, vector<string> array);
    SymbolType determineSymbolType(bool& hasLabel, vector<string> array);
public:
    FirstPass();
    virtual ~FirstPass();
    void processLine(string line);
};

#endif

