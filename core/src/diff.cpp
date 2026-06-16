// morf - core/src/diff.cpp
#include "diff.hpp"

namespace Morf {
    Diff DiffEngine::compare(const Model& base, const Model& target) {
        Diff diff;

        auto baseRefs = buildTemporaryFaceReferences(base);
        auto targetRefs = buildTemporaryFaceReferences(target);
        std::sort(baseRefs.begin(), baseRefs.end());
        std::sort(targetRefs.begin(), targetRefs.end());

        std::size_t i = 0, j = 0;
        while (i < baseRefs.size() && j < targetRefs.size()) {
            const auto& b = baseRefs[i];
            const auto& t = targetRefs[j];

            if (b.canonicalFace < t.canonicalFace) {
                diff.removed.push_back({b.objectIdx, b.faceIdx});
                i++;
            } else if (b.canonicalFace > t.canonicalFace) {
                diff.added.push_back({t.objectIdx, t.faceIdx});
                j++;
            } else {
                diff.common.push_back({b.objectIdx, b.faceIdx});
                i++; j++;
            }
        }
        while (i < baseRefs.size()) {
            diff.removed.push_back({baseRefs[i].objectIdx, baseRefs[i].faceIdx});
            i++;
        }
        while (j < targetRefs.size()) {
            diff.added.push_back({targetRefs[j].objectIdx, targetRefs[j].faceIdx});
            j++;
        }

        diff.hasDiff = !diff.added.empty() || !diff.removed.empty();
        return diff;
    }

    std::vector<TempFaceRef> DiffEngine::buildTemporaryFaceReferences(const Model& model) {
        std::size_t totalFaces = 0;
        for (const auto& obj : model.objects) { totalFaces += obj.faces.size(); }
        std::vector<TempFaceRef> refs;
        refs.reserve(totalFaces);
        
        for (std::size_t o = 0; o < model.objects.size(); ++o) {
            for (std::size_t f = 0; f < model.objects[o].faces.size(); ++f) {
                refs.push_back({
                    model.objects[o].faces[f].getCanonicalForm(model.vertices),
                    o,
                    f
                });
            }
        }
        return refs;
    }
} // namespace Morf