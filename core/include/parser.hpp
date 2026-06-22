/* morf - core/include/parser.hpp
 * Parsing logic
 */
#pragma once
#include "model.hpp"
#include <string>
#include <unordered_map>

namespace Morf {
    class Parser {
        public:
            static Model parse(const std::string& filePath);

        private:
            static std::unordered_map<std::string, Material> parseMtl(const std::string& filePath);
    };
} // namespace Morf