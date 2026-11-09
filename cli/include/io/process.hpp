#pragma once
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace Morf {
    // POSIX shell quoting, including quotes and shell metacharacters in filenames.
    inline std::string shellQuote(const std::string& value) {
        std::string result = "'";
        for (char c : value) result += (c == '\'') ? "'\\''" : std::string(1, c);
        return result + "'";
    }
    inline void runCommand(const std::string& command) {
        if (std::system(command.c_str()) != 0)
            throw std::runtime_error("Command failed: " + command);
    }
}
