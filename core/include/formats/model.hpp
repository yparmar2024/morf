/* morf - core/include/formats/model.hpp
 * Generic model to compute delta and render
 */
#pragma once
#include <array>
#include <cstddef>
#include <vector>

namespace Morf {
    class GenericModel {
        private:
            std::vector<std::array<float, 3>> vertices;
            std::vector<std::vector<std::size_t>> faces;

        public:
            const std::vector<std::array<float, 3>>& getVertices() const { return vertices; }
            const std::vector<std::vector<std::size_t>>& getFaces() const { return faces; }

            void addVertex(const std::array<float, 3>& v) { vertices.push_back(v); }
            void addFace(const std::vector<std::size_t>& f) { faces.push_back(f); }

            std::vector<std::vector<std::size_t>> getCanonicalForm() const;
    };
} // namespace Morf