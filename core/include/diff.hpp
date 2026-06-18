/* morf - core/include/diff.hpp
 * Diffing logic between two models with proper original vertex fetching
 */
#pragma once
#include "model.hpp"
#include <vector>

namespace Morf {
    struct TempFaceRef {
        std::vector<Vertex> canonicalFace;
        std::size_t objectIdx;
        std::size_t faceIdx;

        bool operator==(const TempFaceRef& other) const { return canonicalFace == other.canonicalFace; }
        bool operator<(const TempFaceRef& other) const { return canonicalFace < other.canonicalFace; }
        bool operator>(const TempFaceRef& other) const { return canonicalFace > other.canonicalFace; }
    };

    struct FaceRef {
        std::size_t objectIdx;
        std::size_t faceIdx;
    };

    struct Diff {
        bool hasDiff = false;
        std::vector<FaceRef> common;
        std::vector<FaceRef> added;
        std::vector<FaceRef> removed;
    };

    class DiffEngine {
        public:
            static Diff compare(const Model& base, const Model& target);
        
        private:
            static std::vector<TempFaceRef> buildTemporaryFaceReferences(const Model& model);
    };
} // namespace Morf