/* morf - gui/include/geometry/bounds.hpp
 * Math logic to see if two boundary boxes collide in 3D space
 */
#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace Morf {
    struct AABB {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    AABB getFaceBox(const std::vector<std::size_t>& face, const std::vector<std::array<float, 3>>& vertices);
    bool boxesOverlap(const AABB& a, const AABB& b);
} // namespace Morf