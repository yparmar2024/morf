/* morf - core/include/formats/obj/model.hpp
 * Model for .obj file format
 */
#pragma once
#include <string>
#include <vector>

namespace Morf {
    struct Vertex {
        float x, y, z;
    };

    struct VertexTexture {
        float u, v, w = 0.0f;
    };
    
    struct VertexNormal {
        float x, y, z;
    };

    struct VertexData {
        int vIdx = -1, vtIdx = -1, vnIdx = -1;
    };

    struct Face {
        std::vector<VertexData> vertexData;
    };

    struct Object {
        std::string name;
        std::vector<Face> faces;
    };

    class Model {
        private:
            std::vector<Vertex> vertices;
            std::vector<VertexTexture> textureVertices;
            std::vector<VertexNormal> normalVertices;
            std::vector<Object> objects;

        public:
            const std::vector<Vertex>& getVertices() const { return vertices; }
            const std::vector<VertexTexture>& getTextureVertices() const { return textureVertices; }
            const std::vector<VertexNormal>& getNormalVertices() const { return normalVertices; }
            const std::vector<Object>& getObjects() const { return objects; }

            void addVertex(const Vertex& v) { vertices.push_back(v); }
            void addTextureVertex (const VertexTexture& vt) { textureVertices.push_back(vt); }
            void addNormalVertex (const VertexNormal& vn) { normalVertices.push_back(vn); }
            void addFace(const Face& f) { objects.empty() ? objects.push_back({"default", {f}}) : objects.back().faces.push_back(f); }
            void addObject(const Object& o) { objects.push_back(o); }
    };
} // namespace Morf