#include "SecondPass.h"
#include <iostream>
#include <map>
#include <set>
#include <bitset>
#include <fstream>
#include <iomanip>
using namespace std;

SecondPass::SecondPass(int lastNumber) {
    myRegex =  new RegexParser();
    number = lastNumber;
    locationCounter = 0;
    line = "";
    currentSection = ".undefined";
    
    DataEntry text;
    DataEntry data;
    DataEntry rodata;
    DataEntry bss;
    
    sections.insert(std::make_pair("text", text));
    sections.insert(std::make_pair("data", data));
    sections.insert(std::make_pair("rodata", rodata));
    sections.insert(std::make_pair("bss", bss));

}

SecondPass::~SecondPass() {
    
}

void SecondPass::cleanUpLine(string& line) {
    myRegex->removeWhiteSpacesBeforeAndAfterInstruction(line);
    myRegex->removeMultipleWhiteSpacesInsideInstruction(line);
    myRegex->removeComments(line);
}

vector<string> SecondPass::splitStringBySpace(string line) {
    vector<string> result;
    istringstream iss(line);
    for (string s; iss >> s ; ) {
        myRegex->removeCommas(s);
        result.push_back(s);
        cout << s << endl;
    }
    return result;
}

void SecondPass::processLine(string line) {
    this->line = line;
    vector<string> array;
    if (line == "")
        return;
    cout << endl << "******** " << this->locationCounter << " **** " << this->currentSection << " ******** " << endl ;
    cleanUpLine(this->line);
    array = splitStringBySpace(this->line);
    createDataEntry(array);
}    

void SecondPass::createDataEntry(vector<string> array) {
    bool hasLabel;
    int i;
    SymbolType symbolType = determineSymbolType(hasLabel, array);
    
    switch (symbolType){
        case SECTION:        
            this->locationCounter = 0;
            this->currentSection = array[(hasLabel?1:0)];
            cout << "itsa Sekcija" << endl;
            break;
            
        case INSTRUCTION:
            this->createInstructionDataEntry(hasLabel, array);
            break;
            
        case DIRECTIVE:
            if (array[(hasLabel?1:0)] == ".global")
                for (i = 0; i < (hasLabel?array.size()-2:array.size()-1); i++)
                    globalizeSymbol(array[(hasLabel?i+2:i+1)]);
            else if (this->currentSection != ".undefined")
                this->createDirectiveDataEntry(hasLabel,array);
                    
            break;
            
        default:
            break;
    }
    
    calculateOffsetsAndSizes(hasLabel, symbolType, array);
        
}


SymbolType SecondPass::determineSymbolType(bool& hasLabel, vector<string> array) {
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
    
unsigned short SecondPass::getInstructionCode(string string){
    if (string == "add"){
        return 0;
    } else if (string == "sub") {
        return 1;
    } else if (string == "mul") {
        return 2;
    } else if (string == "div") {
        return 3;
    } else if (string == "cmp") {
        return 4;
    } else if (string == "and") {
        return 5;
    } else if (string == "or") {
        return 6;
    } else if (string == "not") {
        return 7;
    } else if (string == "test") {
        return 8;
    } else if (string == "push") {
        return 9;
    } else if (string == "pop") {
        return 10;
    } else if (string == "call") {
        return 11;
    } else if (string == "iret") {
        return 12;
    } else if (string == "mov") {
        return 13;
    } else if (string == "shl") {
        return 14;
    } else if (string == "shr") {
        return 15;
    }
}
   
unsigned short SecondPass::getConditionCode(string string){
    if (string == "eq") {
        return 0;
    } else if (string == "ne") {
        return 1;
    } else if (string == "gt") {
        return 2;
    } else if (string == "al") {
        return 3;
    }
}

unsigned short SecondPass::getOperandCodeAndData(string string, bool& hasData, unsigned short& data){

    unsigned short type = 0;
    unsigned short number = 0;
    std::string expression;
    
    if (myRegex->regDir(string)) {
        hasData = false;
        type = 1;
        number = stoi(string.substr(string.size()-1));
        
    } else if (myRegex->regInd(string)) {
        hasData = true;
        type = 3;
        number = stoi(string.substr(1,1)); 
        expression = string.substr(3,string.size()-4);
        if (myRegex->immed(expression)) {
            data = stoi(expression);
        } else if (myRegex->memDir(expression)) {
            try{
                data = symbols.at(expression).value;
            } catch (out_of_range exception) {
                throw Error("ERROR: Operand not present in symbol table!");
            }
        }    
        
    } else if (myRegex->memDir(string) && (string != "psw")) {
        try {
            hasData = true;
            type = 2;
            data = symbols.at(string).value;
            
        } catch (out_of_range exception){
            throw Error("ERROR: Operand not present in symbol table!");
        }
        
    } else if (myRegex->immed(string) || (string == "psw")) {
        type = 0;
        if (string == "psw") {
            hasData = false;
            number = 7;
        } else {
            hasData = true;
            number = 0;
            data = stoi(string);
        }

    } else if (myRegex->memStar(string)) {
        
    } else if (myRegex->symValue(string)) {
        try {
             type = 0;
             hasData = true;    
             data = symbols.at(string).value;
        } catch (out_of_range exception){
            throw Error("ERROR: Operand not present in symbol table!");
        }
        
    } else if (myRegex->pcRel(string)) {
        try {
            hasData = true;
            type = 2;
            data = symbols.at(string.substr(1,string.size())).value + this->locationCounter;            
        } catch (out_of_range exception){
            throw Error("ERROR: Operand not present in symbol table!");
        }
    }
    
    return ((type << 3) | number);    
}

void SecondPass::createInstructionDataEntry(bool hasLabel, vector<string> array){

    unsigned short conditionCode = 0;
    unsigned short opCode = 0;
    unsigned short first = 0;
    unsigned short second = 0;
    unsigned short data = 0; 
    
    unsigned short twoByteIns = 0;
    unsigned long fourByteIns = 0;
    
    DataEntry entry;
    
    // [XX------][--------][--------][--------]
    string condition = array[(hasLabel?1:0)].substr(array[(hasLabel?1:0)].size()-2);
    conditionCode = getConditionCode(condition);
    twoByteIns |= (conditionCode << 14);
    
    // [--XXXX--][--------][--------][--------]
    string instruction = array[(hasLabel?1:0)].substr(0,array[(hasLabel?1:0)].size()-2);
    opCode = getInstructionCode(instruction);
    twoByteIns |= (opCode << 10);   
   
    // [------XX][YYY-----][--------][--------]
    string op1;
    bool hasData1 = false;
    if (array.size() >= (hasLabel?3:2)) {
        op1 = array[(hasLabel?2:1)];
        first = getOperandCodeAndData(op1, hasData1, data);
        twoByteIns |= (first << 5);       
    }
    
    // [--------][---XXYYY][--------][--------]
    string op2;
    bool hasData2 = false;
    if (array.size() >= (hasLabel?4:3)){
        op2 = array[(hasLabel?3:2)];
        second = getOperandCodeAndData(op2, hasData2, data);
        twoByteIns |= second;
    }
    
    if (hasData1 != hasData2) {
        
        // [--------][--------][XXXXXXXX][XXXXXXXX]
        fourByteIns = ((fourByteIns | twoByteIns) << 16) | data;
        
        cout << "Creating 4B instruction entry:" << endl;
        cout << "[" << std::bitset<8>(fourByteIns>>24) << "][" << std::bitset<8>(fourByteIns>>16) << "]";
        cout << "[" << std::bitset<8>(fourByteIns>>8) << "][" << std::bitset<8>(fourByteIns) << "]" << endl;

        cout << "   [" << std::hex << (fourByteIns>>24);
        cout << "]       [" << std::hex << ((fourByteIns>>16) & 0b11111111);
        cout << "]       [" << std::hex << ((fourByteIns>>8) & 0b11111111);
        cout << "]       [" << std::hex <<  (fourByteIns & 0b11111111) << "]" << endl;;  
        
        sections.at("text").data.push_back(fourByteIns & 0b11111111);
        sections.at("text").data.push_back(fourByteIns>>8 & 0b11111111);
        sections.at("text").data.push_back(fourByteIns>>16 & 0b11111111);
        sections.at("text").data.push_back(fourByteIns>>24 & 0b11111111);
        
    } else {
        cout << "Creating 2B instruction entry:" << endl;
        cout << "[" << std::bitset<8>(twoByteIns>>8) << "][" << std::bitset<8>(twoByteIns) << "]" << endl;
        cout << "[" << std::hex << (twoByteIns>>8);
        cout << "]\t[" << std::hex <<  (twoByteIns & 0b11111111) << "]" << endl;  
        
        sections.at("text").data.push_back(twoByteIns & 0b11111111);
        sections.at("text").data.push_back(twoByteIns>>8 & 0b11111111);
    }
    
}

bool SecondPass::globalizeSymbol(string string){    
    SymbolEntry entry;
    try {
        symbols.at(string).scope = 'g';
    } catch (out_of_range exception) {
        entry.id = this->number++;
        entry.scope = 'g';
        entry.section = this->currentSection;
        entry.size = 0;
        entry.type = "symbol";
        entry.value = 0;
        symbols.insert(std::make_pair(string, entry));
    }
}


void SecondPass::calculateOffsetsAndSizes(bool hasLabel, SymbolType symbolType, vector<string> array) {
    string ins, dir;
    int instructionSizeInBytes = 0;
    unsigned temp;
    vector<string> parameters = {};
    
    if (symbolType != EMPTY && symbolType != UNDEFINED)
        for (int i = 0; i < (hasLabel?array.size()-2:array.size()-1) ; i++) {
            parameters.push_back(array[hasLabel?i+2:i+1]);
        }

    switch(symbolType){            
        case DIRECTIVE:
            
            dir = array[(hasLabel?1:0)];

            if (dir == ".char") {
                instructionSizeInBytes = 1 * parameters.size();
            } else if (dir == ".word") {
                instructionSizeInBytes = 2 * parameters.size();
            } else if (dir == ".long") {
                instructionSizeInBytes = 4 * parameters.size();
            } else if (dir == ".align") {
                temp = this->locationCounter % stoi(array[(hasLabel?2:1)]);
                if (temp != 0)
                    instructionSizeInBytes += stoi(array[(hasLabel?2:1)]) - temp;
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
                } else if (myRegex->pcRel(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->pcRel(parameters[1])) {
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
                } else if (myRegex->pcRel(parameters[0])) {
                    instructionSizeInBytes = 4;
                }
            } else if (ins == "iret") {
                
            } 
            
            break;
            
        default:
            break;
    }
    
    this->locationCounter += instructionSizeInBytes;
}

void SecondPass::printOutSections() {
    
    ofstream output;
    output.open("example2.txt");
    
    vector<unsigned char>::iterator it; 
    
    output << "#text" << endl;
    for(it = sections.at("text").data.begin(); it != sections.at("text").data.end(); ++it){
        output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
    }
    
    output << endl << "#data" << endl;
    for(it = sections.at("data").data.begin(); it != sections.at("data").data.end(); ++it){
        output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";

    }
    
    output << endl << "#rodata" << endl;
    for(it = sections.at("rodata").data.begin(); it != sections.at("rodata").data.end(); ++it){
        output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
    }
    
    output << endl << "#bss" << endl;
    for(it = sections.at("bss").data.begin(); it != sections.at("bss").data.end(); ++it){
        output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
    }
    
    output.close();

}

void SecondPass::createDirectiveDataEntry(bool hasLabel, vector<string> array) {
    int temp;
    int i,j;
    string dir = array[hasLabel?1:0];
    
    if (dir == ".char") {
        for (i = 0; i < (hasLabel?(array.size()-2):(array.size()-1)) ; i++) {
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back(stoi(array[hasLabel?i+2:i+1]));
        }
    } else if (dir == ".word") {
        for (i = 0; i < (hasLabel?(array.size()-2):(array.size()-1)) ; i++) {
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back(stoi(array[hasLabel?i+2:i+1]) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((stoi(array[hasLabel?i+2:i+1]) >> 8) & 0b11111111);
        }
    } else if (dir == ".long") {
        for (i = 0; i < (hasLabel?(array.size()-2):(array.size()-1)) ; i++) {
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back(stoi(array[hasLabel?i+2:i+1]) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((stoi(array[hasLabel?i+2:i+1]) >> 8) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((stoi(array[hasLabel?i+2:i+1]) >> 16) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((stoi(array[hasLabel?i+2:i+1]) >> 24) & 0b11111111);
        }
            
    } else if (dir == ".skip") {
        for (i = 0; i < stoi(array[hasLabel?2:1]); i++) {
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((int)0);
        }
    } else if (dir == ".align") {
         temp = this->locationCounter % stoi(array[(hasLabel?2:1)]);
         if (temp != 0){
             j = stoi(array[(hasLabel?2:1)]) - temp;
             for (i = 0; i < j; i++) {
                 sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((int)0);
             }
         }
    }
}