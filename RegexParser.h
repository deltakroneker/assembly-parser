#ifndef REGEXPARSER_H
#define	REGEXPARSER_H

#include <iostream>
#include <regex>
#include <string>
#include <vector>
using namespace std;

class RegexParser {
    string regString;    
public:
    RegexParser();
    virtual ~RegexParser();
    
    bool isHexadecimalDigit(string string);
    bool isLabel(string string);
    bool isConditionedInstruction(string string);
    bool isProperlyNamedLabel(string string);
    bool isVariableConstantOrExpression(string string);
        
    bool regDir(string string);   // r1
    bool psw(string string);      // psw
    bool immed(string string);    // 20
    bool regInd(string string);   // r1[20]
    bool memDir(string string);   // x
    bool memStar(string string);  // *20
    bool symValue(string string); // &x
    bool pcRel(string string);    // $x
    
    void removeWhiteSpacesBeforeAndAfterInstruction(string& string);
    void removeMultipleWhiteSpacesInsideInstruction(string& string);
    void removeComments(string& string);
    void removeCommas(string& string);
};

#endif

