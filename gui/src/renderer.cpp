// morf - gui/src/renderer.cpp
#include "renderer.hpp"
#include "rlgl.h"

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

    void Renderer::createModels(const Model& base, const Model& target, const Merge& merge) {
        if (loaded_) unloadModels();
        commonModels_     = buildRaylibModels(base,   merge.common,             LIGHTGRAY);
        keptBaseModels_   = buildRaylibModels(base,   merge.acceptedFromBase,   LIGHTGRAY);
        keptTargetModels_ = buildRaylibModels(target, merge.acceptedFromTarget, LIGHTGRAY);
        addedModels_      = buildRaylibModels(target, merge.added,     { 0, 255, 0, 180 });
        removedModels_    = buildRaylibModels(base,   merge.removed,   { 255, 0, 0, 180 });
        loaded_ = true;
    }

    void Renderer::initLighting() {
        if (commonModels_.empty()) return;
        
        static Shader lightingShader = []() {
            Shader shader = LoadShaderFromMemory(lightingVs, lightingFs);

            float ambient[4]  = { 0.5f, 0.5f, 0.5f, 1.0f };
            float lightDir[3] = { 0.5f, 1.0f, 0.8f };
            float lightCol[4] = { 0.9f, 0.9f, 0.9f, 1.0f };

            SetShaderValue(shader, GetShaderLocation(shader, "ambient"),    ambient,  SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, GetShaderLocation(shader, "lightPos"),   lightDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, GetShaderLocation(shader, "lightColor"), lightCol, SHADER_UNIFORM_VEC4);

            return shader;
        }();

        for (auto& model : commonModels_)     model.materials[0].shader = lightingShader;
        for (auto& model : keptBaseModels_)   model.materials[0].shader = lightingShader;
        for (auto& model : keptTargetModels_) model.materials[0].shader = lightingShader;
    }

    void Renderer::drawModels() const {
        if (!loaded_) return;
        for (auto& model : commonModels_)     DrawModel(model, { 0, 0, 0 }, 1.0f, WHITE);
        for (auto& model : keptBaseModels_)   DrawModel(model, { 0, 0, 0 }, 1.0f, WHITE);
        for (auto& model : keptTargetModels_) DrawModel(model, { 0, 0, 0 }, 1.0f, WHITE);

        BeginBlendMode(BLEND_ALPHA);
        for (auto& model : addedModels_)      DrawModel(model, { 0, 0, 0 }, 1.0f, WHITE);
        for (auto& model : removedModels_)    DrawModel(model, { 0, 0, 0 }, 1.0f, WHITE);
        EndBlendMode();
    }

    void Renderer::drawHighlight(const Model& base, const Model& target, const Merge& merge) const {
        if (!loaded_ || !merge.hasPending()) return;
        
        const Model* srcModel = nullptr;
        FaceRef highlightFace;
        if (merge.selectedIndex < merge.added.size()) {
            srcModel = &target;
            highlightFace = merge.added[merge.selectedIndex];
        } else {
            srcModel = &base;
            highlightFace = merge.removed[merge.selectedIndex - merge.added.size()];
        }

        const auto& obj = srcModel->objects[highlightFace.objectIdx];
        const auto& face = obj.faces[highlightFace.faceIdx];

        std::vector<Vector3> points;
        for (const auto& vData : face.vertexData) {
            const auto& vertex = srcModel->vertices[vData.vIdx];
            points.push_back({ vertex.x, vertex.y, vertex.z });
        }

        Color highlightColor = { 255, 255, 255, 100 };
        for (std::size_t i = 0; i < points.size() - 1; ++i) {
            DrawTriangle3D(points[0], points[i], points[i + 1], highlightColor);
        }

        Color edgeColor = { 255, 255, 255, 255 };
        for (std::size_t i = 0; i < points.size(); ++i) {
            DrawLine3D(points[i], points[(i + 1) % points.size()], edgeColor);
        }
    }

    void Renderer::unloadModels() {
        if (!loaded_) { return; }
        for (auto& model : commonModels_)     UnloadModel(model);
        for (auto& model : keptBaseModels_)   UnloadModel(model);
        for (auto& model : keptTargetModels_) UnloadModel(model);
        for (auto& model : addedModels_)      UnloadModel(model);
        for (auto& model : removedModels_)    UnloadModel(model);

        commonModels_.clear();
        keptBaseModels_.clear();
        keptTargetModels_.clear();
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

            Mesh currMesh = { 0 };
            currMesh.vertexCount   = currVerticeCount;
            currMesh.triangleCount = currTriangleCount;
            currMesh.vertices      = (float*)MemAlloc(vertices.size() * sizeof(float));
            currMesh.indices       = (unsigned short*)MemAlloc(indices.size() * sizeof(unsigned short));
            memcpy(currMesh.vertices, vertices.data(), vertices.size() * sizeof(float));
            memcpy(currMesh.indices,  indices.data(),  indices.size()  * sizeof(unsigned short));

            if (!model.normalVertices.empty()) {
                currMesh.normals = (float*)MemAlloc(normals.size() * sizeof(float));
                memcpy(currMesh.normals, normals.data(), normals.size() * sizeof(float));
            } else {
                currMesh.normals = nullptr;
            }

            UploadMesh(&currMesh, false);
            ::Model raylibModel = LoadModelFromMesh(currMesh);

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

            // Raylib enforces all vertices in meshes to be below USHRT_MAX,
            // so we create a new mesh everytime we surpass that limit
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