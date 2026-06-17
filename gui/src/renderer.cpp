// morf - gui/src/renderer.cpp
#include "raylib.h"
#include "renderer.hpp"

namespace Morf {
    const char* lightingVs = R"(#version 330
    in vec3 vertexPosition;
    in vec3 vertexNormal;
    uniform mat4 mvp;
    uniform mat4 matModel;
    out vec3 fragPosition;
    out vec3 fragNormal;
    void main() {
        fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
        fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
        gl_Position = mvp * vec4(vertexPosition, 1.0);
    }
    )";
    const char* lightingFs = R"(#version 330
    in vec3 fragPosition;
    in vec3 fragNormal;
    uniform vec4 colDiffuse;
    uniform vec4 ambient;
    uniform vec3 lightPos;
    uniform vec4 lightColor;
    out vec4 finalColor;
    void main() {
        vec3 normal = normalize(fragNormal);
        vec3 lightDir = normalize(lightPos);
        float diff = abs(dot(normal, lightDir));
        vec4 diffuse = diff * lightColor * colDiffuse;
        finalColor = ambient * colDiffuse + diffuse;
    }
    )";

    void Renderer::createModels(const Model& base, const Model& target, const Diff& diff) {
        if (loaded_) unloadModels();
        commonModels_  = buildRaylibModels(target, diff.common,  LIGHTGRAY);
        addedModels_   = buildRaylibModels(target, diff.added,   {0,255,0,180});
        removedModels_ = buildRaylibModels(base,   diff.removed, {255,0,0,180});
        loaded_ = true;
    }

    void Renderer::initLighting() {
        if (commonModels_.empty()) return;

        Shader lightingShader = LoadShaderFromMemory(lightingVs, lightingFs);
        float ambient[4]  = { 0.5f, 0.5f, 0.5f, 1.0f };
        float lightDir[3] = { 0.5f, 1.0f, 0.8f };
        float lightCol[4] = { 0.9f, 0.9f, 0.9f, 1.0f };

        SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "ambient"), ambient, SHADER_UNIFORM_VEC4);
        SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "lightPos"), lightDir, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "lightColor"), lightCol, SHADER_UNIFORM_VEC4);

        for (auto& model : commonModels_) model.materials[0].shader = lightingShader;
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
        std::vector<float> normals;
        std::vector<unsigned short> indices;
        int currVerticeCount = 0;
        int currTriangleCount = 0;

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

            if (!model.normalVertices.empty()) {
                mesh.normals = (float*)MemAlloc(normals.size() * sizeof(float));
                memcpy(mesh.normals, normals.data(), normals.size() * sizeof(float));
            } else {
                mesh.normals = nullptr;
            }

            UploadMesh(&mesh, false);
            ::Model raylibModel = LoadModelFromMesh(mesh);

            // If there are no normals, load unlit, otherwise keep default
            if (model.normalVertices.empty()) raylibModel.materials[0].shader = LoadShader(0, 0);
            raylibModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;

            raylibModels.push_back(raylibModel);

            vertices.clear();
            normals.clear();
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
                const auto& vertex = model.vertices[face.vertexData[i].vIdx];
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);

                if (!model.normalVertices.empty() && face.vertexData[i].vnIdx >= 0) {
                    const auto& normal = model.normalVertices[face.vertexData[i].vnIdx];
                    normals.push_back(normal.x);
                    normals.push_back(normal.y);
                    normals.push_back(normal.z);
                }
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