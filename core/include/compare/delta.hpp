/* morf - core/include/compare/delta.hpp
 * Delta object structure for changes
 */
#pragma once
#include <cstddef>
#include <vector>

namespace Morf {
    struct Delta {
        std::vector<std::vector<std::size_t>> common;  // Uses base indices
        std::vector<std::vector<std::size_t>> added;   // Uses target indices
        std::vector<std::vector<std::size_t>> removed; // Uses base indices
    };
} // namespace Morf