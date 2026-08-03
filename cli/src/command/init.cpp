// morf - cli/src/command/diff.cpp
#include "command/init.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Morf {
    int Init::run() {
        std::system("git config diff.morf.command \"sh -c 'morf diff \\\"\\$2\\\" \\\"\\$5\\\"' - \"");
        std::system("git config merge.morf.name \"morf\"");
        std::system("git config merge.morf.driver \"morf merge %O %A %B\"");

        bool needsAppend = true;
        std::ifstream attrCheck(".gitattributes");
        if (attrCheck.is_open()) {
            std::string line;
            while (std::getline(attrCheck, line)) {
                if (line.find("diff=morf") != std::string::npos) {
                    needsAppend = false;
                    break;
                }
            }
            attrCheck.close();
        }

        if (needsAppend) {
            std::ofstream gitattributes(".gitattributes", std::ios::app);
            if (gitattributes.is_open()) {
                gitattributes << "\n*.obj diff=morf merge=morf -text\n";
                gitattributes.close();
            } else {
                throw std::runtime_error("Could not open or create .gitattributes in the current directory.");
            }
        }
        std::cout << "Initialized morf to work in current repository.\n";
        return 0;
    }    
} // namespace Morf