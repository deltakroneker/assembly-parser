#include "RegexParser.h"
#include "Structures.h"
#include <vector>
#include <set>
using namespace std;

RegexParser::RegexParser() {
    this->regString = "//";
}

RegexParser::~RegexParser() {
}


bool RegexParser::isHexadecimalDigit(string string) {
    this->regString = "[0-9]";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::isLabel(string string) {
    this->regString = ".+:$";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::isConditionedInstruction(string string){
    this->regString = "^(";
    std::set<std::string>::iterator it = InstructionNames.begin();
    while (it != InstructionNames.end()){
        if (it != InstructionNames.begin()) {
            this->regString = this->regString + "|" + *it;
        } else {
            this->regString = this->regString + *it;
        }
        it++;
    }
    this->regString = this->regString + ")(";
    it = ConditionNames.begin();
    while (it != ConditionNames.end()){
        if (it != ConditionNames.begin()) {
            this->regString = this->regString + "|" + *it;
        }
        else {
            this->regString = this->regString + *it;
        }
        it++;
    }
    this->regString = this->regString + ")";
        
    return regex_match(string, regex(this->regString));
}

bool RegexParser::isProperlyNamedLabel(string string) {
    this->regString = "^([a-z][a-z0-9_]*):$";
    return regex_match(string, regex(this->regString));
    
}

bool RegexParser::isVariableConstantOrExpression(string string) {
    this->regString = "([a-z][a-z0-9_]*)|([0-9]{1,5})";
    return regex_match(string, regex(this->regString));
}



bool RegexParser::regDir(string string){
    this->regString = "r[0-7]";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::psw(string string){
    this->regString = "psw";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::immed(string string){
    this->regString = "[0-9]{1,5}";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::regInd(string string){
    this->regString = "r[0-7]\\[[0-9]{1,5}\\]|r[0-7]\\[[a-z][a-z0-9_]*\\]";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::memDir(string string){
    this->regString = "[a-z][a-z0-9_]*";
    return regex_match(string, regex(this->regString));
}

bool RegexParser::memStar(string string) {
    this->regString = "\\*[0-9]{1,5}";
    return regex_match(string, regex(this->regString));
}
    
    
bool RegexParser::symValue(string string) {
    this->regString = "&[a-z][a-z0-9_]*";
    return regex_match(string, regex(this->regString));
}


void RegexParser::removeWhiteSpacesBeforeAndAfterInstruction(string& string) {
    this->regString = "^\\s+|\\s+$";
    string = regex_replace(string, regex(this->regString), "");
}

void RegexParser::removeMultipleWhiteSpacesInsideInstruction(string& string) {
    this->regString = "\\s+";
    string = regex_replace(string, regex(this->regString), " ");
}

void RegexParser::removeComments(string& string) {
    this->regString = " ?;.*";
    string = regex_replace(string, regex(this->regString), "");
}

void RegexParser::removeCommas(string& string) {
    this->regString = ",$";
    string = regex_replace(string, regex(this->regString), "");
}

