#pragma once
#include "model.hpp"
#include <vector>

namespace Morf {
    /* Struct for sorting canonical face while storing indices */
    struct TempFaceRef {
        std::vector<Vertex> canonicalFace;
        std::size_t objectIdx;
        std::size_t faceIdx;

        bool operator==(const TempFaceRef& other) const { return canonicalFace == other.canonicalFace; }
        bool operator<(const TempFaceRef& other) const { return canonicalFace < other.canonicalFace; }
        bool operator>(const TempFaceRef& other) const { return canonicalFace > other.canonicalFace; }
    };

    /* Struct for storing purely indices */
    struct FaceRef {
        std::size_t objectIdx;
        std::size_t faceIdx;
    };

    /* Struct for the difference between two models */
    struct Diff {
        bool hasDiff = false;
        std::vector<FaceRef> added;
        std::vector<FaceRef> removed;
        std::vector<FaceRef> common;
    };

    /* DiffEngine object with public compare method */
    class DiffEngine {
        public:
            static Diff compare(const Model& base, const Model& target);
        
        private:
            static std::vector<TempFaceRef> buildTemporaryFaceReferences(const Model& model);
    };
}