#ifndef FIRSTPASS_H
#define	FIRSTPASS_H

#include <iostream>
#include "RegexParser.h"
using namespace std;

class FirstPass {
    
private:
    RegexParser* myRegex;
    void cleanUpLine(string& line);
    void splitStringBySpace(string line);
public:
    FirstPass();
    virtual ~FirstPass();
    
    void processLine(string line);
    
};

#endif

