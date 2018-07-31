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
        
    void removeWhiteSpacesBeforeAndAfterInstruction(string& string);
    void removeMultipleWhiteSpacesInsideInstruction(string& string);
    void removeComments(string& string);
    void removeCommas(string& string);

};

#endif

