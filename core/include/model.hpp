#pragma once
#include <string>
#include <vector>

namespace Polydiff {
    // Struct for each vertex
    struct Vertex {
        float x, y, z;
    };

    // Struct for each vertex's metadata
    struct VertexData {
        int vIdx = -1;
    };

    // Struct for each face
    struct Face {
        std::vector<VertexData> vertices;
    };

    // Struct for each mesh
    struct Mesh {
        std::string name;
        std::vector<Face> faces;
    };

    // Struct for each 3d model
    struct Model {
        std::vector<Vertex> vertices;
        std::vector<Mesh> meshes;
    };
}