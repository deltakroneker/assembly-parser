#ifndef FIRSTPASS_H
#define	FIRSTPASS_H

#include <iostream>
#include "RegexParser.h"
#include "Structures.h"
using namespace std;

class FirstPass {
    
private:
    int number;
    int startAddress;
    unsigned locationCounter;
    string currentSection;
    string line;
    string outputFile;
    bool endDirective;
    RegexParser* myRegex;
    
    void cleanUpLine(string& line);
    vector<string> splitStringBySpace(string line);
    void createSymbolEntry(vector<string> array);
    SymbolType determineSymbolType(bool& hasLabel, vector<string> array);
    void makeSureLabelIsCorrectlyNamed(vector<string> array);
    void makeSureOperandsAreCorrect(bool hasLabel, SymbolType symbolType, vector<string> array);
    void calculateOffsetsAndSizes(bool hasLabel, SymbolType symbolType, vector<string> array);

public:
    FirstPass(string output, string start);
    virtual ~FirstPass();
    void processLine(string line);
    bool hasNoEndDirective();
    void printOutSymbolTable();
    int getNumber();
};

#endif

