/* morf - core/include/morf/compare/mesh_diff.hpp
 * Engine for computing deltas
 */
#pragma once
#include "morf/compare/delta.hpp"
#include "morf/geometry/mesh.hpp"

namespace Morf {
    class MeshDiff {
        public:
            static Delta computeDelta(const PolygonMesh& base, const PolygonMesh& target);
    };
} // namespace Morf