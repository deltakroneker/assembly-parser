#include <iostream>
#include <fstream>
#include "FirstPass.h"
#include "SecondPass.h"
using namespace std;

int main(int argc, char** argv) {

    FirstPass* firstPass = new FirstPass();
    ifstream file(argv[1]);
    string line;

    while (getline(file,line)) {
        firstPass->processLine(line);
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

