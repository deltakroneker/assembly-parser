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
    std::set<std::string>::iterator it = OperationNames.begin();
    while (it != OperationNames.end()){
        if (it != OperationNames.begin()) {
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

