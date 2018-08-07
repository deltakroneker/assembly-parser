#include "FirstPass.h"
#include "RegexParser.h"
#include "Structures.h"
#include <iostream>
using namespace std;

FirstPass::FirstPass() {
    myRegex =  new RegexParser();
    number = 0;
    locationCounter = 0;
    line = "";
    currentSection = ".undefined";
    endDirective = false;
}

FirstPass::~FirstPass() {
    
}

void FirstPass::processLine(string line) {
    this->line = line;
    vector<string> array;
    SymbolEntry entry;

    if (line == ""){
        return;
    } 
            
    cout << endl << "~~~~~~~~ " << this->locationCounter << " ~~~~ " << this->currentSection << " ~~~~~~~ " << endl ;
    
    cleanUpLine(this->line);
    array = splitStringBySpace(this->line);
    createSymbolEntry(entry, array);
    
}
    
void FirstPass::cleanUpLine(string& line) {
    myRegex->removeWhiteSpacesBeforeAndAfterInstruction(line);
    myRegex->removeMultipleWhiteSpacesInsideInstruction(line);
    myRegex->removeComments(line);
}

vector<string> FirstPass::splitStringBySpace(string line) {
    vector<string> result;
    istringstream iss(line);
    for (string s; iss >> s ; ) {
        myRegex->removeCommas(s);
        result.push_back(s);
        cout << s << endl;
    }
    return result;
}

void FirstPass::createSymbolEntry(SymbolEntry& entry, vector<string> array) {
    
    bool hasLabel;
    SymbolType symbolType = determineSymbolType(hasLabel, array);
    
    if (hasLabel)
        makeSureLabelIsCorrectlyNamed(array);

    
    makeSureOperandsAreCorrect(hasLabel, symbolType, array);
    
    switch (symbolType){
        case SECTION:
             cout << "Kreiran simbol SECTION" << endl;
             entry.type = SECTION;
             entry.section.label = hasLabel? array[0] : "";
             entry.section.name = hasLabel? array[1] : array[0];
             entry.number = this->number++;
             entry.size;
             entry.address;
             break;
        case DIRECTIVE:
            cout << "Kreiran simbol DIRECTIVE" << endl;
            entry.type = DIRECTIVE;
            break;
        case INSTRUCTION:
            
            
            cout << "Kreiran simbol OPERATION" << endl;
            entry.type = INSTRUCTION;
            
            
            break;
        case EMPTY:
            cout << "Prazna linija" << endl;
            entry.type = EMPTY;
            break;
        case UNDEFINED:
            cout << "Simbol nije prepoznat" << endl;
            entry.type = UNDEFINED;
            break;
    }
    
    calculateOffsetsAndSizes(hasLabel, symbolType, array);
}

SymbolType FirstPass::determineSymbolType(bool& hasLabel, vector<string> array) {
    vector<string> arrayWithoutLabel;
    
    if (myRegex->isLabel(array[0])) {
        for (int i = 0; i < array.size(); i++){
          if (i != 0) {
              arrayWithoutLabel.push_back(array[i]);
          }
        }
        hasLabel = true;
    } else {
        arrayWithoutLabel = array;
        hasLabel = false;
    }
        
    if (arrayWithoutLabel.size() == 0) {
        return EMPTY;
    } else {
        if (SectionNames.find(arrayWithoutLabel[0]) != SectionNames.end()) {
            return SECTION;
        } else if (DirectiveNames.find(arrayWithoutLabel[0]) != DirectiveNames.end()) {
            return DIRECTIVE;
        } else if (myRegex->isConditionedInstruction(arrayWithoutLabel[0])) {
            return INSTRUCTION;
        } else {
            return UNDEFINED;
        }   
    }
}
    
void FirstPass::makeSureLabelIsCorrectlyNamed(vector<string> array) {

    if (myRegex->isProperlyNamedLabel(array[0]) == false) {
        throw Error("ERROR: Label can only contain lowercase letters a-z, numbers 0-9 and underscore. It must start with a letter.!");
    } else if (myRegex->isConditionedInstruction(array[0].substr(0, array[0].size()-1))
            || (RegisterNames.find(array[0].substr(0, array[0].size()-1)) != RegisterNames.end()) 
            || (InstructionNames.find(array[0].substr(0, array[0].size()-1)) != InstructionNames.end())) {

        throw Error("ERROR: Label cannot be same as reserved words (operation names or register names).");
    }
}

void FirstPass::makeSureOperandsAreCorrect(bool hasLabel, SymbolType symbolType, vector<string> array){
    
    string insWithCondition;
    string ins;
    string dir;
    int i = 0;
    
    switch(symbolType){
        
        case SECTION:
            if (hasLabel && array.size() > 2 || !hasLabel && array.size() > 1) 
                throw Error("ERROR: Sections cannot have operands!");
            break;
            
        case DIRECTIVE:
            dir = array[(hasLabel?1:0)];
            
            
             if (dir == ".end") {
                this->endDirective = true;
                throw End();
            } else {
                for (i = 0; i < (hasLabel?array.size()-2:array.size()-1) ; i++) {
                    if (dir == ".char" || dir == ".word" || dir == ".long") {
                        if (!myRegex->isVariableConstantOrExpression(array[(hasLabel?i+2:i+1)]))
                            throw Error("ERROR: .char .word or .long directive parameters are not valid.");
                    } else if (dir == ".skip") {
                        if (!myRegex->immed(array[(hasLabel?i+2:i+1)]) || (array.size() > (hasLabel?4:3)))
                            throw Error("ERROR: .skip directive parameters are not valid.");
                    } else if (dir == ".align") {
                        if (!myRegex->immed(array[(hasLabel?i+2:i+1)]) || (array.size() > (hasLabel?6:5)))
                            throw Error("ERROR: .align directive parameters are not valid.");
                    } else if (dir == ".global") {
                        if (!myRegex->memDir(array[(hasLabel?i+2:i+1)]))
                            throw Error("ERROR: .global directive parameters are not valid.");
                    }
                } 
            }
            break;
            
        case INSTRUCTION:
            insWithCondition =  array[(hasLabel?1:0)];
            ins = array[(hasLabel?1:0)].substr(0,array[(hasLabel?1:0)].size()-2);
            
            if (!myRegex->isConditionedInstruction(insWithCondition)){
                throw Error("ERROR: Instruction operand is not conforming to conditional naming protocol. Add conditional keyword.");
            }

            if (array.size() > (hasLabel?2:1)){
                if (myRegex->immed(array[hasLabel?2:1])) {
                    throw Error("ERROR: Destination operand cannot be a number");
                }
            }
            break;
            
        case EMPTY:
            break;
            
        case UNDEFINED:
            break;
    }
    
}

void FirstPass::calculateOffsetsAndSizes(bool hasLabel, SymbolType symbolType, vector<string> array) {
    string ins, dir;
    int instructionSizeInBytes = 0;
    vector<string> parameters = {};
    
    if (this->currentSection == ".undefined"){
        if (symbolType == DIRECTIVE || symbolType == INSTRUCTION){
            if (array[hasLabel?1:0] != ".global"){
                throw Error("ERROR: All directives (except .global) and instructions must be inside sections!");
            }   
        }
    } else {
        if (symbolType == DIRECTIVE){
            if (array[hasLabel?1:0] == ".global"){
                throw Error("ERROR: .global instruction must be outside all sections!");
            }   
        }
    }
    
    
    if (symbolType != EMPTY && symbolType != UNDEFINED)
        for (int i = 0; i < (hasLabel?array.size()-2:array.size()-1) ; i++) {
            parameters.push_back(array[hasLabel?i+2:i+1]);
        }

    switch(symbolType){
        case SECTION:
            this->currentSection = array[(hasLabel?1:0)];
            this->locationCounter = 0;
            break;
            
        case DIRECTIVE:
            
            dir = array[(hasLabel?1:0)];

            if (dir == ".char") {
                instructionSizeInBytes = 1 * parameters.size();
            } else if (dir == ".word") {
                instructionSizeInBytes = 2 * parameters.size();
            } else if (dir == ".long") {
                instructionSizeInBytes = 4 * parameters.size();
            } else if (dir == ".align") {
                
            } else if (dir == ".skip") {
                instructionSizeInBytes += stoi(parameters[(hasLabel?1:0)]);
            } else if (dir == ".global") {
                
            }
            
            break;
            
        case INSTRUCTION:
            ins = array[(hasLabel?1:0)].substr(0,array[(hasLabel?1:0)].size()-2);
            
            if (ins == "add" || ins == "sub" || ins == "mul" || ins == "div" || ins == "and" || ins == "or" || ins == "not" || ins == "cmp" || ins == "test" || ins == "mov" || ins == "shl" || ins == "shr") {
                
                if (myRegex->regDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 2;
                } else if (myRegex->psw(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->psw(parameters[0]) && myRegex->immed(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->regInd(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regInd(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->immed(parameters[1])){
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memStar(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memStar(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->symValue(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->symValue(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                }        
            } else if (ins == "push" || ins == "pop") {
                if (myRegex->regDir(parameters[0])) {
                    instructionSizeInBytes = 2;
                }
            } else if (ins == "call") {
                if (myRegex->regDir(parameters[0])) {
                    instructionSizeInBytes = 2;
                } else if (myRegex->regInd(parameters[0])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memDir(parameters[0])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->immed(parameters[0])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->psw(parameters[0])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memStar(parameters[0])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->symValue(parameters[0])) {
                    instructionSizeInBytes = 4;
                }
            } else if (ins == "iret") {
                
            }
            
            break;
            
        case EMPTY:
            break;
            
        case UNDEFINED:
            break;
    }
    
    this->locationCounter += instructionSizeInBytes;
}


bool FirstPass::hasNoEndDirective() {
    return !this->endDirective;
}