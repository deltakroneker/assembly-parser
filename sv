
            if (ins == "add" || ins == "sub" || ins == "mul" || ins == "div") {
                if (myRegex->regDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 2;
                } else if (myRegex->regDir(parameters[0]) && myRegex->regInd(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regInd(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->immed(parameters[1])){
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memStar(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memStar(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->symValue(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->symValue(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                }       
            } else if (ins == "and" || ins == "or" || ins == "mov") {
                if (myRegex->regDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 2;
                } else if (myRegex->psw(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->psw(parameters[0]) && myRegex->immed(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->regInd(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regInd(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->immed(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memStar(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memStar(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->symValue(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->symValue(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                }
                
            } else if (ins == "test") {
                if (myRegex->regDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 2;
                } else if (myRegex->psw(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->psw(parameters[0]) && myRegex->immed(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->immed(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memStar(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memStar(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } 
                 
            } else if (ins == "cmp") {
                if (myRegex->regDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 2;
                } else if (myRegex->regDir(parameters[0]) && myRegex->regInd(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regInd(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                }  else if (myRegex->regDir(parameters[0]) && myRegex->memDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memDir(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->memStar(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->memStar(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->regDir(parameters[0]) && myRegex->symValue(parameters[1])) {
                    instructionSizeInBytes = 4;
                } else if (myRegex->symValue(parameters[0]) && myRegex->regDir(parameters[1])) {
                    instructionSizeInBytes = 4;
                }   
                 