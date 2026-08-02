// morf - core/src/compare/engine.cpp
#include "compare/engine.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace Morf {
    Delta Engine::computeDelta(const GenericModel& base, const GenericModel& target) {
        const auto isLess = [&](const std::vector<std::size_t>& baseFace, const std::vector<std::array<float, 3>>& baseVertices,
                                const std::vector<std::size_t>& targetFace, const std::vector<std::array<float, 3>>& targetVertices) -> bool
        {
            if (baseFace.size() != targetFace.size()) return baseFace.size() < targetFace.size();

            for (std::size_t i = 0; i < baseFace.size(); ++i) {
                if (baseVertices[baseFace[i]] < targetVertices[targetFace[i]]) return true;
                if (baseVertices[baseFace[i]] > targetVertices[targetFace[i]]) return false;
            }
            return false;
        };

        Delta delta;
        auto baseCanon = base.getCanonicalForm();
        auto targetCanon = target.getCanonicalForm();
        const auto& bVerts = base.getVertices();
        const auto& tVerts = target.getVertices();

        std::size_t i = 0, j = 0;
        while (i < baseCanon.size() && j < targetCanon.size()) {
            auto b = baseCanon[i];
            auto t = targetCanon[j];

            if (isLess(b, bVerts, t, tVerts)) {
                delta.removed.push_back(b);
                i++;
            } else if (isLess(t, tVerts, b, bVerts)) {
                delta.added.push_back(t);
                j++;
            } else {
                delta.common.push_back(b);
                i++; j++;
            }
        }
        while (i < baseCanon.size()) delta.removed.push_back(baseCanon[i++]);
        while (j < targetCanon.size()) delta.added.push_back(targetCanon[j++]);
        
        return delta;
    }
} // namespace Morf