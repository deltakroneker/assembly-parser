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
            
    cout << endl << "~~~~~~~~~~~~" << endl ;
    
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
    
    if (hasLabel){
        cout << "hasLabel " << array[0] << endl;

        if (myRegex->isProperlyNamedLabel(array[0]) == false) {
            throw Error("ERROR: Label can only contain lowercase letters a-z, numbers 0-9 and underscore. It must start with a letter.!");
        } else if (myRegex->isConditionedInstruction(array[0].substr(0, array[0].size()-1))
                || (RegisterNames.find(array[0].substr(0, array[0].size()-1)) != RegisterNames.end()) 
                || (OperationNames.find(array[0].substr(0, array[0].size()-1)) != OperationNames.end())) {
        
            throw Error("ERROR: Label cannot be same as reserved words (operation names or register names).");
        }
    }
    
    
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
        case OPERATION:
            cout << "Kreiran simbol OPERATION" << endl;
            entry.type = OPERATION;
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
            return OPERATION;
        } else {
            return UNDEFINED;
    }
        
}
    
    
}