// morf - gui/src/renderer.cpp
#include "raylib.h"
#include "renderer.hpp"

namespace Morf {
    void Renderer::createModels(const Model& base, const Model& target, const Diff& diff) {
        if (loaded_) unloadModels();
        commonModels_  = buildRaylibModels(target, diff.common,  LIGHTGRAY);
        addedModels_   = buildRaylibModels(target, diff.added,   {0,255,0,180});
        removedModels_ = buildRaylibModels(base,   diff.removed, {255,0,0,180});
        loaded_ = true;
    }

    void Renderer::drawModels() const {
        if (!loaded_) return;
        for (auto& model : commonModels_)  DrawModel(model, {0,0,0}, 1.0f, WHITE);

        BeginBlendMode(BLEND_ALPHA);
        for (auto& model : addedModels_)   DrawModel(model, {0,0,0}, 1.0f, WHITE);
        for (auto& model : removedModels_) DrawModel(model, {0,0,0}, 1.0f, WHITE);
        EndBlendMode();
    }

    void Renderer::unloadModels() {
        if (!loaded_) { return; }
        for (auto& model : commonModels_)  UnloadModel(model);
        for (auto& model : addedModels_)   UnloadModel(model);
        for (auto& model : removedModels_) UnloadModel(model);

        commonModels_.clear();
        addedModels_.clear();
        removedModels_.clear();
        loaded_ = false;
    }

    std::vector<::Model> Renderer::buildRaylibModels(const Model& model, const std::vector<FaceRef>& faces, Color color) const {
        std::vector<::Model> raylibModels;

        // Temporary mesh accumulators
        std::vector<float> vertices;
        std::vector<unsigned short> indices;
        int currVerticeCount = 0;
        int currTriangleCount  = 0;

        auto createMesh = [&]() {
            if (currVerticeCount == 0) return;

            // Current mesh attributes
            Mesh mesh = { 0 };
            mesh.vertexCount   = currVerticeCount;
            mesh.triangleCount = currTriangleCount;
            mesh.vertices = (float*)MemAlloc(vertices.size() * sizeof(float));
            mesh.indices  = (unsigned short*)MemAlloc(indices.size() * sizeof(unsigned short));
            memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
            memcpy(mesh.indices,  indices.data(),  indices.size()  * sizeof(unsigned short));
            UploadMesh(&mesh, false);

            ::Model raylibModel = LoadModelFromMesh(mesh);

            // TODO: Apply vertex normals here for rendering later
            raylibModel.materials[0].shader = LoadShader(0, 0);
            raylibModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;

            raylibModels.push_back(raylibModel);

            vertices.clear();
            indices.clear();
            currVerticeCount  = 0;
            currTriangleCount = 0;
        };

        for (const auto& ref : faces) {
            const auto& face = model.objects[ref.objectIdx].faces[ref.faceIdx];
            int n = (int)face.vertexData.size();
            if (n < 3) continue;

            // Raylib enforces all vertices in meshes to be below USHRT_MAX
            if (currVerticeCount + n > USHRT_MAX) createMesh();

            unsigned short start = (unsigned short)currVerticeCount;
            for (int i = 0; i < n; ++i) {
                const auto& vertex= model.vertices[face.vertexData[i].vIdx];
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);
            }

            for (int i = 1; i < n - 1; ++i) {
                indices.push_back(start);
                indices.push_back(start + (unsigned short)(i));
                indices.push_back(start + (unsigned short)(i + 1));
            }

            currVerticeCount += n;
            currTriangleCount += (n - 2);
        }

        createMesh();
        return raylibModels;
    }
} // namespace Morf