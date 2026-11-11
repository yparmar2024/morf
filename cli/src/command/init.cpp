// morf - cli/src/command/diff.cpp
#include "command/init.hpp"
#include "io/process.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Morf {
    int Init::run() {
        runCommand("git config diff.morf.command \"sh -c 'morf diff \\\"\\$2\\\" \\\"\\$5\\\"' - \"");
        runCommand("git config merge.morf.name \"morf\"");
        runCommand("git config merge.morf.driver \"morf merge \\\"%O\\\" \\\"%A\\\" \\\"%B\\\"\"");

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
            if (attrCheck.bad()) throw std::runtime_error("Failed to read .gitattributes.");
            attrCheck.close();
        }

        if (needsAppend) {
            std::ofstream gitattributes(".gitattributes", std::ios::app);
            if (gitattributes.is_open()) {
                gitattributes << "\n*.obj diff=morf merge=morf -text\n";
                gitattributes.close();
                if (!gitattributes) throw std::runtime_error("Failed to write .gitattributes.");
            } else {
                throw std::runtime_error("Could not open or create .gitattributes in the current directory.");
            }
        }
        std::cout << "Initialized morf to work in current repository.\n";
        return 0;
    }
} // namespace Morf