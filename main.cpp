#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "FirstPass.h"
#include "SecondPass.h"
#include "Structures.h"
using namespace std;

int main(int argc, char** argv) {

    FirstPass* firstPass;
    SecondPass* secondPass;
    ifstream file(argv[1]);
    string line;

    firstPass = new FirstPass();
    while (getline(file,line)) {
        try {
            firstPass->processLine(line);
        } catch (Error error){
            errors.push_back(error.getMessage());
        } catch (End end) {
            break;
        }
    }
    
    if (errors.size() > 0){
        vector<string>::iterator it = errors.begin();
        while (it != errors.end()){
            cout << *it << endl;
            it++;
        }
        file.close();
        return 1;
    } else if (firstPass->hasNoEndDirective()){
        file.close();
        return 1;
    }
    
    file.clear();
    file.seekg(0,file.beg);
    
    secondPass = new SecondPass(firstPass->getNumber());
    while (getline(file,line)) {
        try {
            secondPass->processLine(line);
        } catch (Error error){
            errors.push_back(error.getMessage());
        }
    }
    
    if (errors.size() > 0){
        vector<string>::iterator it = errors.begin();
        while (it != errors.end()){
            cout << *it << endl;
            it++;
        }
        file.close();
        return 1;
    }
    
    file.close();
    
    firstPass->printOutSymbolTable();
    secondPass->printOutSections();
    
    return 0;
}
