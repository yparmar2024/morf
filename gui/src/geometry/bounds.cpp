// morf - gui/src/geometry/bounds.cpp
#include "geometry/bounds.hpp"

namespace Morf {
    AABB getFaceBox(const std::vector<std::size_t>& face, const std::vector<std::array<float, 3>>& vertices) {
        AABB box = { 1e9, 1e9, 1e9, -1e9, -1e9, -1e9 };
        for (std::size_t idx : face) {
            const auto& v = vertices[idx];
            if (v[0] < box.minX) box.minX = v[0];
            if (v[1] < box.minY) box.minY = v[1];
            if (v[2] < box.minZ) box.minZ = v[2];

            if (v[0] > box.maxX) box.maxX = v[0];
            if (v[1] > box.maxY) box.maxY = v[1];
            if (v[2] > box.maxZ) box.maxZ = v[2];
        }
        return box;
    }
    bool boxesOverlap(const AABB& a, const AABB& b) {
        return (a.minX <= b.maxX && a.maxX >= b.minX) &&
               (a.minY <= b.maxY && a.maxY >= b.minY) &&
               (a.minZ <= b.maxZ && a.maxZ >= b.minZ);
    }
} // namespace Morf