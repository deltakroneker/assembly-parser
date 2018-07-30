#ifndef SECONDPASS_H
#define	SECONDPASS_HT

#include <iostream>
using namespace std;

class SecondPass {
    
public:
    SecondPass();
    virtual ~SecondPass();
    
    void processLine(string line);

};

#endif

