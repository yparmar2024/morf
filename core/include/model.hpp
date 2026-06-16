/* morf - core/include/model.hpp
 * Model logic per attribute of a generic model
 */
#pragma once
#include <string>
#include <vector>

namespace Morf {
    struct Vertex {
        float x, y, z;

        bool operator==(const Vertex& other) const { return x == other.x && y == other.y && z == other.z; }
        bool operator<(const Vertex& other) const {
            if (x != other.x) return x < other.x;
            if (y != other.y) return y < other.y;
            return z < other.z;
        }
        bool operator>(const Vertex& other) const {
            if (x != other.x) return x > other.x;
            if (y != other.y) return y > other.y;
            return z > other.z;
        }
    };

    struct VertexTexture {
        float u, v, w = 0.0f;
    };
    
    struct VertexNormal {
        float x, y, z;
    };

    struct VertexData {
        int vIdx = -1;
        int vtIdx = -1;
        int vnIdx = -1;
    };

    struct Face {
        std::vector<VertexData> vertexData;

        std::vector<Vertex> getCanonicalForm(const std::vector<Vertex>& vertices) const {
            const std::size_t n = vertexData.size();
            std::vector<Vertex> faceVertices(n);
            for (std::size_t i = 0; i < n; ++i) {
                faceVertices[i] = vertices[vertexData[i].vIdx];
            }

            std::size_t minIdx = 0;
            for (std::size_t i = 1; i < n; ++i) {
                if (faceVertices[i] < faceVertices[minIdx]) minIdx = i;
            }

            std::vector<Vertex> canonicalFaceForm(n);
            for (std::size_t i = 0; i < n; ++i) {
                canonicalFaceForm[i] = faceVertices[(minIdx + i) % n];
            }
            return canonicalFaceForm;
        }
    };

    struct Object {
        std::string name;
        std::vector<Face> faces;

        std::vector<std::vector<Vertex>> getCanonicalForm(const std::vector<Vertex>& vertices) const {
            const std::size_t n = faces.size();
            std::vector<std::vector<Vertex>> canonicalObjectForm;
            canonicalObjectForm.reserve(n);
            for (const auto& face : faces) {
                canonicalObjectForm.push_back(face.getCanonicalForm(vertices));
            }

            std::sort(canonicalObjectForm.begin(), canonicalObjectForm.end());
            return canonicalObjectForm;
        }
    };

    struct Model {
        std::vector<Vertex> vertices;
        std::vector<VertexTexture> textureVertices;
        std::vector<VertexNormal> normalVertices;
        std::vector<Object> objects;
    };
} // namespace Morf