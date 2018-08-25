#include "SecondPass.h"
#include <iostream>
#include <map>
#include <set>
#include <bitset>
#include <fstream>
#include <iomanip>
using namespace std;

SecondPass::SecondPass(string output, string start, int lastNumber) {
    myRegex =  new RegexParser();
    number = lastNumber;
    locationCounter = stoi(start);
    startAddress = stoi(start);
    outputFile = output;
    line = "";
    currentSection = ".undefined";
    
    sections.insert(std::make_pair("text", DataEntry()));
    sections.insert(std::make_pair("data", DataEntry()));
    sections.insert(std::make_pair("rodata", DataEntry()));
    sections.insert(std::make_pair("bss", DataEntry()));
    
    relocations.insert(std::make_pair("text", RelocationList()));
    relocations.insert(std::make_pair("data", RelocationList()));
    relocations.insert(std::make_pair("rodata", RelocationList()));
    relocations.insert(std::make_pair("bss", RelocationList()));
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
            this->locationCounter = startAddress;
            this->currentSection = array[(hasLabel?1:0)];
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
    
    std::string p1,p2;
    bool plus = true;
    
    try {
        if (myRegex->regDir(string)) {
            cout << "regdir" << endl;

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
            } else if (myRegex->memDir(expression) && !myRegex->isExpression(expression)) {
                data = symbols.at(expression).value;
                if (this->currentSection == symbols.at(expression).section) {
                    createRelocationEntry(0, 0 , expression, "R_386_16", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(expression).scope == 'g'){
                        createRelocationEntry(0, 0 , expression, "R_386_16", symbols.at(expression).id);
                    } else {
                        createRelocationEntry(0, 0 , expression, "R_386_16", symbols.at(symbols.at(expression).section).id);
                    }
                }
            } else if (myRegex->isExpression(expression)) {
                if (expression.find('+') || expression.find('-')){

                    if ((expression.find('+') != 0) && (expression.find('+') != (int)0xffffffff)){
                        p1 = expression.substr(0, expression.find('+'));
                        p2 = expression.substr(expression.find('+')+1);
                    }
                    if ((expression.find('-') != 0) && (expression.find('-') != (int)0xffffffff)){
                        p1 = expression.substr(0, expression.find('-'));
                        p2 = expression.substr(expression.find('-')+1);
                        plus = false;
                    }

                    if (myRegex->immed(p1) && myRegex->immed(p2))
                        data = plus ? stoi(p1)+stoi(p2) : stoi(p1)-stoi(p2);
                    else if (myRegex->immed(p1) && myRegex->memDir(p2))
                        data = plus ? stoi(p1)+symbols.at(p2).value : stoi(p1)-symbols.at(p2).value;
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        data = plus? symbols.at(p1).value+stoi(p2) : symbols.at(p1).value-stoi(p2) ;
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2))
                        data = plus? symbols.at(p1).value+symbols.at(p2).value : symbols.at(p1).value-symbols.at(p2).value;

                    if (myRegex->immed(p1) && myRegex->memDir(p2))
                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(symbols.at(p1).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2)){
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(symbols.at(p1).section).id);
                            }
                        }

                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    }
                }
            }

        } else if (myRegex->memDir(string) && (string != "psw")) {
            hasData = true;
            type = 2;
                
            if (myRegex->isExpression(string)) {
                if (string.find('+') || string.find('-')){

                    if ((string.find('+') != 0) && (string.find('+') != (int)0xffffffff)){
                        p1 = string.substr(0, string.find('+'));
                        p2 = string.substr(string.find('+')+1);
                    }
                    if ((string.find('-') != 0) && (string.find('-') != (int)0xffffffff)){
                        p1 = string.substr(0, string.find('-'));
                        p2 = string.substr(string.find('-')+1);
                        plus = false;
                    }

                    if (myRegex->immed(p1) && myRegex->immed(p2))
                        data = plus ? stoi(p1)+stoi(p2) : stoi(p1)-stoi(p2);
                    else if (myRegex->immed(p1) && myRegex->memDir(p2))
                        data = plus ? stoi(p1)+symbols.at(p2).value : stoi(p1)-symbols.at(p2).value;
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        data = plus? symbols.at(p1).value+stoi(p2) : symbols.at(p1).value-stoi(p2) ;
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2))
                        data = plus? symbols.at(p1).value+symbols.at(p2).value : symbols.at(p1).value-symbols.at(p2).value;

                    if (myRegex->immed(p1) && myRegex->memDir(p2))
                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(symbols.at(p1).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2)){
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(0, 0 , p1, "R_386_16", symbols.at(symbols.at(p1).section).id);
                            }
                        }

                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(0, 0 , p2, "R_386_16", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    }
                }
            } else {
                data = symbols.at(string).value;

                if (this->currentSection == symbols.at(string).section) {
                    createRelocationEntry(0, 0 , string, "R_386_16", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(string).scope == 'g'){
                        createRelocationEntry(0, 0 , string, "R_386_16", symbols.at(string).id);
                    } else {
                        createRelocationEntry(0, 0 , string, "R_386_16", symbols.at(symbols.at(string).section).id);
                    }
                }
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
            hasData = true;
            type = 2;
            data = stoi(string.substr(1,string.size()-1));

        } else if (myRegex->symValue(string)) {
             expression = string.substr(1,string.size()-1);

             type = 0;
             hasData = true;   
             data = symbols.at(expression).value;

             if (this->currentSection == symbols.at(expression).section) {
                    createRelocationEntry(0, 0 , expression, "R_386_16", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(expression).scope == 'g'){
                        createRelocationEntry(0, 0 , expression, "R_386_16", symbols.at(expression).id);
                    } else {
                        createRelocationEntry(0, 0 , expression, "R_386_16", symbols.at(symbols.at(expression).section).id);
                    }
                }

        } else if (myRegex->pcRel(string)) {
            hasData = true;
            type = 2;
            expression = string.substr(1,string.size());
            data = symbols.at(expression).value + this->locationCounter;   
            if (this->currentSection != symbols.at(expression).section) {
                if (symbols.at(expression).scope == 'g'){
                    createRelocationEntry(0, 0 , expression, "R_386_PCR_16", symbols.at(expression).id);
                } else {
                    createRelocationEntry(0, 0 , expression, "R_386_PCR_16", symbols.at(symbols.at(expression).section).id);
                }
            }
        }
        
    } catch (out_of_range exception) {
        throw Error("ERROR: Operand not present in symbol table!");
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
    
    // jmp is an exception that is handled differently
    if (instruction == "jmp"){
        handleJumpIns(hasLabel, array);
        return;
    }
    
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
        
    if (instruction != "cmp" && instruction != "test" && instruction != "push" && instruction != "call" && instruction != "iret" && instruction != "ret") {
        try {
            if (myRegex->memDir(op1)) {
                if (symbols.at(op1).section == ".rodata")
                    throw Error("ERROR: Cannot modify symbols from read-only section .rodata!");
            } else if (myRegex->pcRel(op1))
                 if (symbols.at(op1.substr(1,op1.size()-1)).section == ".rodata")
                    throw Error("ERROR: Cannot modify symbols from read-only section .rodata!");
        } catch (out_of_range exception) {
        }
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
        entry.section = "/";
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
            } else if (ins == "call" || ins == "jmp") {
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
                instructionSizeInBytes = 2;
            } else if (ins == "ret") {
                instructionSizeInBytes = 2;
            }
            
            break;
            
        default:
            break;
    }
    
    this->locationCounter += instructionSizeInBytes;
}

void SecondPass::printOutSections() {
    int i = 1;
    ofstream output;
    output.open(outputFile, std::ios::app);
    
    vector<unsigned char>::iterator it; 
    
    output << endl << "#text" << endl;
    if (sections.at("text").data.size() == 0) 
        output << "/" << endl;
    else
        for(it = sections.at("text").data.begin(); it != sections.at("text").data.end(); ++it){
            output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
            if ((i % 16) == 0 && (i != 0)) {
                i++;
                output << endl;
            } else {
                i++;
            }
        }
    
    i = 1;
    output << endl << endl << "#data" << endl;
    if (sections.at("data").data.size() == 0) 
        output << "/" << endl;
    else 
        for(it = sections.at("data").data.begin(); it != sections.at("data").data.end(); ++it){
            output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
            if ((i % 16) == 0 && (i != 0)) {
                i++;
                output << endl;
            } else {
                i++;
            }
        }
    
    i = 1;
    output << endl << endl << "#rodata" << endl;
    if (sections.at("rodata").data.size() == 0) 
        output << "/" << endl;
    else 
        for(it = sections.at("rodata").data.begin(); it != sections.at("rodata").data.end(); ++it){
            output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
            if ((i % 16) == 0 && (i != 0)) {
                i++;
                output << endl;
            } else {
                i++;
            }
        }
    
    i = 1;
    output << endl << endl << "#bss" << endl;
    if (sections.at("bss").data.size() == 0) 
        output << "/" << endl;
    else 
        for(it = sections.at("bss").data.begin(); it != sections.at("bss").data.end(); ++it){
            output << std::hex << uppercase << setw(2) << setfill('0') << (int)*it <<  " ";
            if ((i % 16) == 0 && (i != 0)) {
                i++;
                output << endl;
            } else {
                i++;
            }
        }
    
    output.close();

}

void SecondPass::createDirectiveDataEntry(bool hasLabel, vector<string> array) {
    int temp;
    int i,j;
    string dir = array[hasLabel?1:0];
    string operand;
    string p1,p2;
    bool plus = true;

    
    if (dir == ".char") {
        for (i = 0; i < (hasLabel?(array.size()-2):(array.size()-1)) ; i++) {
            
            operand = array[hasLabel?i+2:i+1];
            if (myRegex->immed(operand)){
                temp = stoi(operand);
            } else if (myRegex->isSectionName(operand)) {
                temp = (int)symbols.at(operand).value;
            } else if (myRegex->memDir(operand) && !myRegex->isExpression(operand)){
                temp = (int)symbols.at(operand).value;
            } else if (myRegex->isExpression(operand)) {
                if (operand.find('+') || operand.find('-')){

                    if ((operand.find('+') != 0) && (operand.find('+') != (int)0xffffffff)){
                        p1 = operand.substr(0, operand.find('+'));
                        p2 = operand.substr(operand.find('+')+1);
                    }
                    if ((operand.find('-') != 0) && (operand.find('-') != (int)0xffffffff)){
                        p1 = operand.substr(0, operand.find('-'));
                        p2 = operand.substr(operand.find('-')+1);
                        plus = false;
                    }

                    if (myRegex->immed(p1) && myRegex->immed(p2))
                        temp = plus ? stoi(p1)+stoi(p2) : stoi(p1)-stoi(p2);
                    else if (myRegex->immed(p1) && myRegex->memDir(p2))
                        temp = plus ? stoi(p1)+symbols.at(p2).value : stoi(p1)-symbols.at(p2).value;
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        temp = plus? symbols.at(p1).value+stoi(p2) : symbols.at(p1).value-stoi(p2) ;
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2))
                        temp = plus? symbols.at(p1).value+symbols.at(p2).value : symbols.at(p1).value-symbols.at(p2).value;

                    if (myRegex->immed(p1) && myRegex->memDir(p2))
                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(1, i, p2, "R_386_8", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(1, i , p2, "R_386_8", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(1, i , p2, "R_386_8", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(1, i , p1, "R_386_8", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(1, i , p1, "R_386_8", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(1, i , p1, "R_386_8", symbols.at(symbols.at(p1).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2)){
                        if ((this->currentSection == symbols.at(p1).section) != (this->currentSection == symbols.at(p2).section)) {
                            if (this->currentSection == symbols.at(p1).section) {
                                createRelocationEntry(1, i , p1, "R_386_8", symbols.at(this->currentSection).id);
                            } else {
                                if (symbols.at(p1).scope == 'g'){
                                    createRelocationEntry(1, i , p1, "R_386_8", symbols.at(p1).id);
                                } else {
                                    createRelocationEntry(1, i , p1, "R_386_8", symbols.at(symbols.at(p1).section).id);
                                }
                            }

                            if (this->currentSection == symbols.at(p2).section) {
                                createRelocationEntry(1, i , p2, "R_386_8", symbols.at(this->currentSection).id);
                            } else {
                                if (symbols.at(p2).scope == 'g'){
                                    createRelocationEntry(1, i , p2, "R_386_8", symbols.at(p2).id);
                                } else {
                                    createRelocationEntry(1, i , p2, "R_386_8", symbols.at(symbols.at(p2).section).id);
                                }
                            }
                        }
                    }
                }
            } else {
                return;
            }
    
            
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back(temp);
            
            if (myRegex->memDir(operand) && !myRegex->isExpression(operand)){
                if (this->currentSection == symbols.at(operand).section) {
                    createRelocationEntry(1, i , operand, "R_386_8", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(operand).scope == 'g'){
                        createRelocationEntry(1, i , operand, "R_386_8", symbols.at(operand).id);
                    } else {
                        createRelocationEntry(1, i , operand, "R_386_8", symbols.at(symbols.at(operand).section).id);
                    }
                }
            } else if (myRegex->isSectionName(operand)) {
                 if (this->currentSection == symbols.at(operand).section) {
                    createRelocationEntry(1, i , operand, "R_386_8", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(operand).scope == 'g'){
                        createRelocationEntry(1, i , operand, "R_386_8", symbols.at(operand).id);
                    } else {
                        createRelocationEntry(1, i , operand, "R_386_8", symbols.at(symbols.at(operand).section).id);
                    }
                }
            }
        }
    } else if (dir == ".word") {
        for (i = 0; i < (hasLabel?(array.size()-2):(array.size()-1)) ; i++) {
            
            operand = array[hasLabel?i+2:i+1];
            if (myRegex->immed(operand)){
                temp = stoi(operand);
            } else if (myRegex->isSectionName(operand)) {
                temp = (int)symbols.at(operand).value;
            } else if (myRegex->memDir(operand) && !myRegex->isExpression(operand)){
                temp = (int)symbols.at(operand).value;
            } else if (myRegex->isExpression(operand)) {
                if (operand.find('+') || operand.find('-')){

                    if ((operand.find('+') != 0) && (operand.find('+') != (int)0xffffffff)){
                        p1 = operand.substr(0, operand.find('+'));
                        p2 = operand.substr(operand.find('+')+1);
                    }
                    if ((operand.find('-') != 0) && (operand.find('-') != (int)0xffffffff)){
                        p1 = operand.substr(0, operand.find('-'));
                        p2 = operand.substr(operand.find('-')+1);
                        plus = false;
                    }

                    if (myRegex->immed(p1) && myRegex->immed(p2))
                        temp = plus ? stoi(p1)+stoi(p2) : stoi(p1)-stoi(p2);
                    else if (myRegex->immed(p1) && myRegex->memDir(p2))
                        temp = plus ? stoi(p1)+symbols.at(p2).value : stoi(p1)-symbols.at(p2).value;
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        temp = plus? symbols.at(p1).value+stoi(p2) : symbols.at(p1).value-stoi(p2) ;
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2))
                        temp = plus? symbols.at(p1).value+symbols.at(p2).value : symbols.at(p1).value-symbols.at(p2).value;

                    if (myRegex->immed(p1) && myRegex->memDir(p2))
                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(2, i, p2, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(2, i, p2, "R_386_16", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(2, i, p2, "R_386_16", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(2, i, p1, "R_386_16", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(2, i, p1, "R_386_16", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(2, i, p1, "R_386_16", symbols.at(symbols.at(p1).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2)){
                        if ((this->currentSection == symbols.at(p1).section) != (this->currentSection == symbols.at(p2).section)) {
                            if (this->currentSection == symbols.at(p1).section) {
                                createRelocationEntry(2, i, p1, "R_386_16", symbols.at(this->currentSection).id);
                            } else {
                                if (symbols.at(p1).scope == 'g'){
                                    createRelocationEntry(2, i, p1, "R_386_16", symbols.at(p1).id);
                                } else {
                                    createRelocationEntry(2, i, p1, "R_386_16", symbols.at(symbols.at(p1).section).id);
                                }
                            }

                            if (this->currentSection == symbols.at(p2).section) {
                                createRelocationEntry(2, i, p2, "R_386_16", symbols.at(this->currentSection).id);
                            } else {
                                if (symbols.at(p2).scope == 'g'){
                                    createRelocationEntry(2, i, p2, "R_386_16", symbols.at(p2).id);
                                } else {
                                    createRelocationEntry(2, i, p2, "R_386_16", symbols.at(symbols.at(p2).section).id);
                                }
                            }
                        }
                    }
                }
            } else {
                return;
            }
            
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((temp) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((temp >> 8) & 0b11111111);
        
            if (myRegex->memDir(operand) && !myRegex->isExpression(operand)){
                if (this->currentSection == symbols.at(operand).section) {
                    createRelocationEntry(2, i, operand, "R_386_16", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(operand).scope == 'g'){
                        createRelocationEntry(2, i, operand, "R_386_16", symbols.at(operand).id);
                    } else {
                        createRelocationEntry(2, i, operand, "R_386_16", symbols.at(symbols.at(operand).section).id);
                    }
                }
            } else if (myRegex->isSectionName(operand)) {
                 if (this->currentSection == symbols.at(operand).section) {
                    createRelocationEntry(2, i , operand, "R_386_16", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(operand).scope == 'g'){
                        createRelocationEntry(2, i , operand, "R_386_16", symbols.at(operand).id);
                    } else {
                        createRelocationEntry(2, i , operand, "R_386_16", symbols.at(symbols.at(operand).section).id);
                    }
                }
            }
            
        }
    } else if (dir == ".long") {
        for (i = 0; i < (hasLabel?(array.size()-2):(array.size()-1)) ; i++) {
            
            operand = array[hasLabel?i+2:i+1];
            if (myRegex->immed(operand)){
                temp = stoi(operand);
            } else if (myRegex->isSectionName(operand)) {
                temp = (int)symbols.at(operand).value;
            } else if (myRegex->memDir(operand) && !myRegex->isExpression(operand)){
                temp = (int)symbols.at(operand).value;
            } else if (myRegex->isExpression(operand)) {
                if (operand.find('+') || operand.find('-')){

                    if ((operand.find('+') != 0) && (operand.find('+') != (int)0xffffffff)){
                        p1 = operand.substr(0, operand.find('+'));
                        p2 = operand.substr(operand.find('+')+1);
                    }
                    if ((operand.find('-') != 0) && (operand.find('-') != (int)0xffffffff)){
                        p1 = operand.substr(0, operand.find('-'));
                        p2 = operand.substr(operand.find('-')+1);
                        plus = false;
                    }

                    if (myRegex->immed(p1) && myRegex->immed(p2))
                        temp = plus ? stoi(p1)+stoi(p2) : stoi(p1)-stoi(p2);
                    else if (myRegex->immed(p1) && myRegex->memDir(p2))
                        temp = plus ? stoi(p1)+symbols.at(p2).value : stoi(p1)-symbols.at(p2).value;
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        temp = plus? symbols.at(p1).value+stoi(p2) : symbols.at(p1).value-stoi(p2) ;
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2))
                        temp = plus? symbols.at(p1).value+symbols.at(p2).value : symbols.at(p1).value-symbols.at(p2).value;

                    if (myRegex->immed(p1) && myRegex->memDir(p2))
                        if (this->currentSection == symbols.at(p2).section) {
                            createRelocationEntry(4, i, p2, "R_386_32", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p2).scope == 'g'){
                                createRelocationEntry(4, i, p2, "R_386_32", symbols.at(p2).id);
                            } else {
                                createRelocationEntry(4, i, p2, "R_386_32", symbols.at(symbols.at(p2).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->immed(p2))
                        if (this->currentSection == symbols.at(p1).section) {
                            createRelocationEntry(4, i, p1, "R_386_32", symbols.at(this->currentSection).id);
                        } else {
                            if (symbols.at(p1).scope == 'g'){
                                createRelocationEntry(4, i, p1, "R_386_32", symbols.at(p1).id);
                            } else {
                                createRelocationEntry(4, i, p1, "R_386_32", symbols.at(symbols.at(p1).section).id);
                            }
                        }
                    else if (myRegex->memDir(p1) && myRegex->memDir(p2)){
                        if ((this->currentSection == symbols.at(p1).section) != (this->currentSection == symbols.at(p2).section)) {
                            if (this->currentSection == symbols.at(p1).section) {
                                createRelocationEntry(4, i, p1, "R_386_32", symbols.at(this->currentSection).id);
                            } else {
                                if (symbols.at(p1).scope == 'g'){
                                    createRelocationEntry(4, i, p1, "R_386_32", symbols.at(p1).id);
                                } else {
                                    createRelocationEntry(4, i, p1, "R_386_32", symbols.at(symbols.at(p1).section).id);
                                }
                            }

                            if (this->currentSection == symbols.at(p2).section) {
                                createRelocationEntry(4, i, p2, "R_386_32", symbols.at(this->currentSection).id);
                            } else {
                                if (symbols.at(p2).scope == 'g'){
                                    createRelocationEntry(4, i, p2, "R_386_32", symbols.at(p2).id);
                                } else {
                                    createRelocationEntry(4, i, p2, "R_386_32", symbols.at(symbols.at(p2).section).id);
                                }
                            }
                        }
                    }
                }
            } else {
                return;
            }
            
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((temp) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((temp >> 8) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((temp >> 16) & 0b11111111);
            sections.at(this->currentSection.substr(1,currentSection.size()-1)).data.push_back((temp >> 24) & 0b11111111);
        
            if (myRegex->memDir(operand) && !myRegex->isExpression(operand)){
                if (this->currentSection == symbols.at(operand).section) {
                    createRelocationEntry(4, i, operand, "R_386_32", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(operand).scope == 'g'){
                        createRelocationEntry(4, i, operand, "R_386_32", symbols.at(operand).id);
                    } else {
                        createRelocationEntry(4, i, operand, "R_386_32", symbols.at(symbols.at(operand).section).id);
                    }
                }
            } else if (myRegex->isSectionName(operand)) {
                 if (this->currentSection == symbols.at(operand).section) {
                    createRelocationEntry(4, i , operand, "R_386_32", symbols.at(this->currentSection).id);
                } else {
                    if (symbols.at(operand).scope == 'g'){
                        createRelocationEntry(4, i , operand, "R_386_32", symbols.at(operand).id);
                    } else {
                        createRelocationEntry(4, i , operand, "R_386_32", symbols.at(symbols.at(operand).section).id);
                    }
                }
            }
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


void SecondPass::printOutRelocations() {
    ofstream output;
    output.open(outputFile, std::ios::app);
    
    vector<RelocationEntry>::iterator it; 
        
    output << endl << "#text_relocation" << endl;
    if (relocations.at("text").relData.size() == 0)
        output << "/" << endl;
    else
        for(it = relocations.at("text").relData.begin(); it != relocations.at("text").relData.end(); ++it){
            output << std::hex << (*it).value << "\t " << (*it).type << " \t" << (*it).id << endl;
        }
        
    output << endl << "#data_relocation" << endl;
    if (relocations.at("data").relData.size() == 0)
        output << "/" << endl;
    else
        for(it = relocations.at("data").relData.begin() ; it != relocations.at("data").relData.end(); ++it){
            output << std::hex << (*it).value << "\t " << (*it).type << " \t" << (*it).id << endl;
        }
    
    output << endl << "#rodata_relocation" << endl;
    if (relocations.at("rodata").relData.size() == 0)
        output << "/" << endl;
    else
        for(it = relocations.at("rodata").relData.begin() ; it != relocations.at("rodata").relData.end(); ++it){
            output << std::hex << (*it).value << "\t " << (*it).type << " \t" << (*it).id << endl;
        }
    
    output << endl << "#bss_relocation" << endl;
    if (relocations.at("bss").relData.size() == 0)
        output << "/" << endl;
    else
        for(it = relocations.at("bss").relData.begin() ; it != relocations.at("bss").relData.end(); ++it){
            output << std::hex << (*it).value << "\t " << (*it).type << " \t" << (*it).id << endl;
        }
    
    output.close();
}

void SecondPass::createRelocationEntry(int size, int position, string symbol, string type, unsigned id) {
    RelocationEntry entry;
        
    entry.value = this->locationCounter - startAddress + size*position;
    entry.type = type;
    entry.id = id;

    cout << "@RELOC CREATED: " << entry.value << " " << entry.type << " " << entry.id << endl;
    
    relocations.at(this->currentSection.substr(1,currentSection.size()-1)).relData.push_back(entry);

}

void SecondPass::handleJumpIns(bool hasLabel, vector<string> array){
    
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
               
    // [------XX][YYY-----][--------][--------]
    string op1;
    string op2;
    bool hasData = false;
    
    if (array.size() >= (hasLabel?3:2)) {
        op1 = array[(hasLabel?2:1)];
        
        if (myRegex->pcRel(op1) || myRegex->memDir(op1)) {
            
            if (myRegex->memDir(op1)){
                opCode = 13;
                twoByteIns |= (opCode << 10);   

                first = ((1 << 3) | 7);    
                twoByteIns |= (first << 5);       

                second = ((0 << 3) | 0);
                twoByteIns |= second;

                hasData = true;
                data = symbols.at(op1).value;
                
                  // [--------][--------][XXXXXXXX][XXXXXXXX]
                fourByteIns = ((fourByteIns | twoByteIns) << 16) | data;

                cout << "Creating 4B JMP instruction entry:" << endl;
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
                opCode = 0;
                twoByteIns |= (opCode << 10);   

                first = ((1 << 3) | 7);    
                twoByteIns |= (first << 5);       

                second = ((0 << 3) | 0);
                twoByteIns |= second;

                hasData = true;
                data = this->locationCounter - symbols.at(op1.substr(1, op1.size()-1)).value;                
                  // [--------][--------][XXXXXXXX][XXXXXXXX]
                fourByteIns = ((fourByteIns | twoByteIns) << 16) | data;

                cout << "Creating 4B JMP PCREL instruction entry:" << endl;
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
                
                 if (this->currentSection != symbols.at(op1.substr(1, op1.size()-1)).section) {
                    if (symbols.at(op1.substr(1, op1.size()-1)).scope == 'g'){
                        createRelocationEntry(0, 0 , op1.substr(1, op1.size()-1), "R_386_PCR_16", symbols.at(op1.substr(1, op1.size()-1)).id);
                    } else {
                        createRelocationEntry(0, 0 , op1.substr(1, op1.size()-1), "R_386_PCR_16", symbols.at(symbols.at(op1.substr(1, op1.size()-1)).section).id);
                    }
                 }
            }

            
        } else {
            return;
        }
        
    }
    
}

