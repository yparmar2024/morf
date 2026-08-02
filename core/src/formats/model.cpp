// morf - core/src/formats/model.cpp
#include "formats/model.hpp"

namespace Morf {
    std::vector<std::vector<std::size_t>> GenericModel::getCanonicalForm() const {
        std::vector<std::vector<std::size_t>> canonicalFaces = faces;

        for (auto& face : canonicalFaces) {
            auto minIter = std::min_element(face.begin(), face.end(),
                [this](const std::size_t& i, const std::size_t& j) -> bool {
                    const auto& vI = vertices[i];
                    const auto& vJ = vertices[j];

                    if (vI[0] != vJ[0]) return vI[0] < vJ[0];
                    if (vI[1] != vJ[1]) return vI[1] < vJ[1];
                    return vI[2] < vJ[2];
                }
            );
            std::rotate(face.begin(), minIter, face.end());
        }

        std::sort(canonicalFaces.begin(), canonicalFaces.end(),
            [this](const std::vector<std::size_t>& a, const std::vector<std::size_t>& b) -> bool {
                if (a.size() != b.size()) return a.size() < b.size();
                for (std::size_t i = 0; i < a.size(); ++i) {
                    const auto& vA = vertices[a[i]];
                    const auto& vB = vertices[b[i]];

                    if (vA[0] != vB[0]) return vA[0] < vB[0];
                    if (vA[1] != vB[1]) return vA[1] < vB[1];
                    if (vA[2] != vB[2]) return vA[2] < vB[2];
                }
                return false;
            }
        );
        return canonicalFaces;
    }
} // namespace Morf