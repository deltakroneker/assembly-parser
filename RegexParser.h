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
        
    void removeWhiteSpacesBeforeAndAfterInstruction(string& string);
    void removeMultipleWhiteSpacesInsideInstruction(string& string);
    void removeComments(string& string);
    void removeCommas(string& string);

};

#endif

