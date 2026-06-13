#pragma once
#include <string>
#include <vector>

namespace Morf {
    /* Struct for each vertex */
    struct Vertex {
        float x, y, z;
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
    };

    /* Struct for each object */
    struct Object {
        std::string name;
        std::vector<Face> faces;
    };

    /* Struct for each 3d model */
    struct Model {
        std::vector<Vertex> vertices;
        std::vector<VertexTexture> textureVertices;
        std::vector<VertexNormal> normalVertices;
        std::vector<Object> objects;
    };
}