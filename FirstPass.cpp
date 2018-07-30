#include "FirstPass.h"
#include "RegexParser.h"
#include <iostream>
using namespace std;

FirstPass::FirstPass() {
    myRegex =  new RegexParser();
}

FirstPass::~FirstPass() {
    
}

void FirstPass::processLine(string line) {
    string newLine = line;
    
    cout << "DIRTY " << newLine << endl;
    cleanUpLine(newLine);
    cout << "CLEAN " << newLine << endl << endl;

    splitStringBySpace(newLine);
}
    
void FirstPass::cleanUpLine(string& line) {
    myRegex->removeWhiteSpacesBeforeAndAfterInstruction(line);
    myRegex->removeMultipleWhiteSpacesInsideInstruction(line);
    myRegex->removeComments(line);
}

void FirstPass::splitStringBySpace(string line) {
    std::vector<std::string> result;
    std::istringstream iss(line);
    for (std::string s; iss >> s ; ) {
        myRegex->removeCommas(s);
        cout << "ONESTRING " << s << endl;
        result.push_back(s);
    }
}