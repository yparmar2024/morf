/* morf - core/include/command/merge.hpp
 * Merge command specific logic
 */
#pragma once
#include <string>

namespace Morf {
    class Merge {
        private:
            static std::string getExtension(const std::string& filePath);
        
        public:
            static int run(const std::string& baseFilePath, const std::string& ourFilePath, const std::string& theirFilePath);
    };
}