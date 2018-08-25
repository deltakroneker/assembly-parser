#ifndef SECONDPASS_H
#define	SECONDPASS_HT

#include <iostream>
#include "RegexParser.h"
#include "Structures.h"
using namespace std;

class SecondPass {
    
private:
    unsigned locationCounter;
    int number;
    int startAddress;
    string currentSection;
    string line;
    string outputFile;
    RegexParser* myRegex;
    
    void cleanUpLine(string& line);
    vector<string> splitStringBySpace(string line);
    void createDataEntry(vector<string> array);
    void createInstructionDataEntry(bool hasLabel, vector<string> array);
    SymbolType determineSymbolType(bool& hasLabel, vector<string> array);
    unsigned short getInstructionCode(string string);
    unsigned short getConditionCode(string string);
    unsigned short getOperandCodeAndData(string string, bool& hasData, unsigned short& data);
    bool globalizeSymbol(string string);
    void calculateOffsetsAndSizes(bool hasLabel, SymbolType symbolType, vector<string> array);
    void createDirectiveDataEntry(bool hasLabel, vector<string> array);
    void createRelocationEntry(int size, int position, string symbol, string type, unsigned id);
    void handleJumpIns(bool hasLabel, vector<string> array);

public:
    SecondPass(string output, string start, int lastNumber);
    virtual ~SecondPass();
    void processLine(string line);
    void printOutSections();
    void printOutRelocations();
};

#endif

