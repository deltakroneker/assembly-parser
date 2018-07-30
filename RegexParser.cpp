#include "RegexParser.h"
#include <vector>
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

