#include "raylib.h"
#include "renderer.hpp"

namespace Morf {
    /* Method to load the difference meshes */
    void Renderer::createMeshes(const Model& base, const Model& target, const Diff& diff) {
        // If the meshes are loaded, unload them first
        if (loaded_) unloadMeshes();

        // Build all three meshes
        commonMesh_  = buildMesh(target, diff.common);
        addedMesh_   = buildMesh(target, diff.added);
        removedMesh_ = buildMesh(base,   diff.removed);

        // Load all the material defaults and map it to our colors
        greyMat_ = LoadMaterialDefault();
        greyMat_.maps[MATERIAL_MAP_DIFFUSE].color = LIGHTGRAY;
        greenMat_ = LoadMaterialDefault();
        greenMat_.maps[MATERIAL_MAP_DIFFUSE].color = { 0, 255, 0, 100 };
        redMat_ = LoadMaterialDefault();
        redMat_.maps[MATERIAL_MAP_DIFFUSE].color = { 255, 0, 0, 100 };

        // Set loaded to true
        loaded_ = true;
    }

    /* Method to draw each mesh */
    void Renderer::drawMeshes() const {
        // If meshes aren't loaded, don't draw
        if (!loaded_) return;

        // Identity matrix for transformation
        Matrix identity = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Draw common mesh, if it exists, in solid color
        if (commonMesh_.triangleCount > 0) { DrawMesh(commonMesh_,  greyMat_,  identity); }

        // Draw added and removed meshes, if it exists, in transparent color
        BeginBlendMode(BLEND_ALPHA);
        if (addedMesh_.triangleCount > 0) { DrawMesh(addedMesh_,   greenMat_, identity); }
        if (removedMesh_.triangleCount > 0) { DrawMesh(removedMesh_, redMat_,   identity); }
        EndBlendMode();
    }

    /* Method to unload all resources */
    void Renderer::unloadMeshes() {
        // If meshes aren't loaded, no need to unload
        if (!loaded_) return;

        // Unload all meshes
        UnloadMesh(commonMesh_);
        UnloadMesh(addedMesh_);
        UnloadMesh(removedMesh_);

        // Zero out meshes
        commonMesh_ = { 0 };
        addedMesh_ = { 0 };
        removedMesh_ = { 0 };

        // Set loaded to false
        loaded_ = false;
    }

    /* Method to build a singular mesh */
    Mesh Renderer::buildMesh(const Model& model, const std::vector<FaceRef>& faces) const {
        // Declare mesh to build
        Mesh mesh = { 0 };

        // Count the total number of vertices and triangles for Raylib rendering
        for (const auto& faceRef : faces) {
            // Compute n
            const auto& face = model.objects[faceRef.objectIdx].faces[faceRef.faceIdx];
            int n = static_cast<int>(face.vertexData.size());
            if (n < 3) continue;

            // Update vertex count and triangle count
            mesh.vertexCount += n;
            mesh.triangleCount += (n - 2);
        }

        // If it has no vertices or triangles, then return mesh
        if (mesh.vertexCount == 0 || mesh.triangleCount == 0) {
            return mesh;
        }

        // Compute space to allocate
        mesh.vertices = static_cast<float*>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
        mesh.indices = static_cast<unsigned short*>(MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short)));

        // Declare next free vertex and triangle index
        int vertexIdx = 0;
        int triangleIdx = 0;

        // Populate vertices and triangle indices
        for (const auto& faceRef : faces) {
            // Fetch the face and the number of vertices
            const auto& face = model.objects[faceRef.objectIdx].faces[faceRef.faceIdx];
            int n = static_cast<int>(face.vertexData.size());
            if (n < 3) continue;

            // Fetch base index for this face
            unsigned short startIdx = static_cast<unsigned short>(vertexIdx);

            // Copy vertex positions
            for (std::size_t i = 0; i < n; ++i) {
                const auto& vertex = model.vertices[face.vertexData[i].vIdx];
                mesh.vertices[vertexIdx * 3]     = vertex.x;
                mesh.vertices[vertexIdx * 3 + 1] = vertex.y;
                mesh.vertices[vertexIdx * 3 + 2] = vertex.z;
                ++vertexIdx;
            }

            // Copy triangle indices
            for (std::size_t i = 1; i < n - 1; ++i) {
                mesh.indices[triangleIdx * 3]     = startIdx;
                mesh.indices[triangleIdx * 3 + 1] = startIdx + static_cast<unsigned short>(i);
                mesh.indices[triangleIdx * 3 + 2] = startIdx + static_cast<unsigned short>(i + 1);
                ++triangleIdx;
            }
        }

        // If the vertex count exceeds the size for unsigned short, log it to prevent segmentation fault
        if (mesh.vertexCount > USHRT_MAX) {
            TraceLog(LOG_ERROR, "Mesh has too many vertices for 16-bit indices");
            UnloadMesh(mesh);
            return { 0 };
        }

        // Upload the mesh and return it
        UploadMesh(&mesh, false);
        return mesh;
    }
}