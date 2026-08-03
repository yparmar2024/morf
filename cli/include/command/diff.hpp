/* morf - cli/include/command/diff.hpp
 * Diff command specific logic
 */
#pragma once
#include <string>

namespace Morf {
    class Diff {
        private:
            static std::string getExtension(const std::string& filePath);
        
        public:
            static int run(const std::string& baseFilePath, const std::string& targetFilePath);
    };
} // namespace Morf