#pragma once
#include "model.hpp"
#include <string>

namespace Morf {
    /* Parser object with public parse method */
    class Parser {
        public:
            static Model parse(const std::string& filePath);
    };
}