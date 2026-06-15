#include "diff.hpp"

namespace Morf {
    /* Method to compare two models */
    Diff DiffEngine::compare(const Model& base, const Model& target) {
        // Initialize a diff for the two models
        Diff diff;

        // Fetch temporary face references and then sort them by canonical faces
        auto baseRefs = buildTemporaryFaceReferences(base);
        auto targetRefs = buildTemporaryFaceReferences(target);
        std::sort(baseRefs.begin(), baseRefs.end());
        std::sort(targetRefs.begin(), targetRefs.end());

        // Declare pointers for base and target and populate added and removed faces
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

        // Populate the remaining faces to its respective vector
        while (i < baseRefs.size()) {
            diff.removed.push_back({baseRefs[i].objectIdx, baseRefs[i].faceIdx});
            i++;
        }
        while (j < targetRefs.size()) {
            diff.added.push_back({targetRefs[j].objectIdx, targetRefs[j].faceIdx});
            j++;
        }

        // Change hasDiff boolean and return the Diff object
        diff.hasDiff = !diff.added.empty() || !diff.removed.empty();
        return diff;
    }

    /* Helper method to build face references vector */
    std::vector<TempFaceRef> DiffEngine::buildTemporaryFaceReferences(const Model& model) {
        // Declare total faces, compute total faces and populate temporaryFaceReferences
        std::size_t totalFaces = 0;
        for (const auto& obj : model.objects) { totalFaces += obj.faces.size(); }
        std::vector<TempFaceRef> refs;
        refs.reserve(totalFaces);
        
        // Populate the face references with object and face indices
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
}