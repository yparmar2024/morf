#pragma once
#include <string>
#include <vector>

namespace Morf {
    /* Struct for each vertex */
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

    /* Struct for each vertex texture */
    struct VertexTexture {
        float u, v, w = 0.0f;
    };
    
    /* Struct for each vertex normal */
    struct VertexNormal {
        float x, y, z;
    };

    /* Struct for each vertex's metadata */
    struct VertexData {
        int vIdx = -1;
        int vtIdx = -1;
        int vnIdx = -1;
    };

    /* Struct for each face */
    struct Face {
        std::vector<VertexData> vertexData;

        std::vector<Vertex> getCanonicalForm(const std::vector<Vertex>& vertices) const {
            // Declare size n, and populate face verticess
            const std::size_t n = vertexData.size();
            std::vector<Vertex> faceVertices(n);
            for (std::size_t i = 0; i < n; ++i) {
                faceVertices[i] = vertices[vertexData[i].vIdx];
            }

            // Find the smallest vertex
            std::size_t minIdx = 0;
            for (std::size_t i = 1; i < n; ++i) {
                if (faceVertices[i] < faceVertices[minIdx]) minIdx = i;
            }

            // Build canonical form by starting from min index
            std::vector<Vertex> canonicalFaceForm(n);
            for (std::size_t i = 0; i < n; ++i) {
                canonicalFaceForm[i] = faceVertices[(minIdx + i) % n];
            }
            return canonicalFaceForm;
        }
    };

    /* Struct for each object */
    struct Object {
        std::string name;
        std::vector<Face> faces;

        std::vector<std::vector<Vertex>> getCanonicalForm(const std::vector<Vertex>& vertices) const {
            // Declare size n and populate canonical face forms
            const std::size_t n = faces.size();
            std::vector<std::vector<Vertex>> canonicalObjectForm;
            canonicalObjectForm.reserve(n);
            for (const auto& face : faces) {
                canonicalObjectForm.push_back(face.getCanonicalForm(vertices));
            }

            // Sort the canonical face forms
            std::sort(canonicalObjectForm.begin(), canonicalObjectForm.end());
            return canonicalObjectForm;
        }
    };

    /* Struct for each 3d model */
    struct Model {
        std::vector<Vertex> vertices;
        std::vector<VertexTexture> textureVertices;
        std::vector<VertexNormal> normalVertices;
        std::vector<Object> objects;

        std::vector<std::vector<Vertex>> getFlatCanonicalFaces() const {
            // Declare size n, compute n and populate flat canonical faces
            std::size_t n = 0;
            for (const auto& obj : objects) { n += obj.faces.size(); }
            std::vector<std::vector<Vertex>> flatCanonicalFaces;
            flatCanonicalFaces.reserve(n);

            // Populate flat canonical faces
            for (const auto& obj : objects) {
                for (const auto& face : obj.faces) {
                    flatCanonicalFaces.push_back(face.getCanonicalForm(vertices));
                }
            }
            return flatCanonicalFaces;
        }
    };
}