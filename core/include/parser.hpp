/* morf - core/include/parser.hpp
 * Parsing logic
 */
#pragma once
#include "model.hpp"
#include <string>

namespace Morf {
    class Parser {
        public:
            static Model parse(const std::string& filePath);
    };
} // namespace Morf