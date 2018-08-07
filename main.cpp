#include <iostream>
#include <fstream>
#include <vector>
#include "FirstPass.h"
#include "SecondPass.h"
#include "Structures.h"
using namespace std;

int main(int argc, char** argv) {

    FirstPass* firstPass = new FirstPass();
    ifstream file(argv[1]);
    string line;

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
        return 1;
    } else if (firstPass->hasNoEndDirective()){
        return 1;
    }
   
    
    /*
    file.clear();
    file.seekg(0,file.beg);
    
    while (getline(file,line)) {
        secondPass->processLine(line);
    }
    */
    
    return 0;
}

