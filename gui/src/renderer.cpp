// morf - gui/src/renderer.cpp
#include "renderer.hpp"
#include "rlgl.h"
#include "raymath.h"

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
        commonModel_     = buildRaylibModel(base,   merge.common,             LIGHTGRAY);
        keptBaseModel_   = buildRaylibModel(base,   merge.acceptedFromBase,   LIGHTGRAY);
        keptTargetModel_ = buildRaylibModel(target, merge.acceptedFromTarget, LIGHTGRAY);
        addedModel_      = buildRaylibModel(target, merge.added,     { 0, 255, 0, 180 });
        removedModel_    = buildRaylibModel(base,   merge.removed,   { 255, 0, 0, 180 });
        loaded_ = true;
    }

    void Renderer::initLighting() {
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

        auto applyShader = [&](::Model& model) {
            for (std::size_t i = 0; i < model.materialCount; ++i) {
                model.materials[i].shader = lightingShader;
            }
        };
        
        applyShader(commonModel_);
        applyShader(keptBaseModel_);
        applyShader(keptTargetModel_);
    }

    void Renderer::drawModels() const {
        if (!loaded_) return;
        DrawModel(commonModel_,     { 0, 0, 0 }, 1.0f, WHITE);
        DrawModel(keptBaseModel_,   { 0, 0, 0 }, 1.0f, WHITE);
        DrawModel(keptTargetModel_, { 0, 0, 0 }, 1.0f, WHITE);

        BeginBlendMode(BLEND_ALPHA);
        DrawModel(addedModel_,      { 0, 0, 0 }, 1.0f, WHITE);
        DrawModel(removedModel_,    { 0, 0, 0 }, 1.0f, WHITE);
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

        if (points.size() < 3) return;

        Color highlightColor = { 255, 255, 255, 100 };
        for (std::size_t i = 1; i < points.size() - 1; ++i) {
            DrawTriangle3D(points[0], points[i], points[i + 1], highlightColor);
        }

        Color edgeColor = { 255, 255, 255, 255 };
        for (std::size_t i = 0; i < points.size(); ++i) {
            DrawLine3D(points[i], points[(i + 1) % points.size()], edgeColor);
        }
    }

    void Renderer::unloadModels() {
        if (!loaded_) { return; }
        UnloadModel(commonModel_);
        UnloadModel(keptBaseModel_);
        UnloadModel(keptTargetModel_);
        UnloadModel(addedModel_);
        UnloadModel(removedModel_);

        commonModel_     = { 0 };
        keptBaseModel_   = { 0 };
        keptTargetModel_ = { 0 };
        addedModel_      = { 0 };
        removedModel_    = { 0 };

        loaded_ = false;
    }

    ::Model Renderer::buildRaylibModel(const Model& model, const std::vector<FaceRef>& faces, Color color) const {
        ::Model raylibModel = { 0 };
        std::vector<Mesh> meshes;

        // Temporary mesh accumulators
        int vertexCount = 0;
        int triangleCount = 0;
        std::vector<float> vertices;
        std::vector<float> texcoords;
        std::vector<float> normals;
        std::vector<unsigned short> indices;

        auto computeFaceNormal = [&](const Face& face) -> Vector3 {
            Vector3 normal = { 0.0f, 0.0f, 0.0f };
            const std::size_t n = face.vertexData.size();

            for (std::size_t i = 0; i < n; ++i) {
                const auto& curr = model.vertices[face.vertexData[i].vIdx];
                const auto& next = model.vertices[face.vertexData[(i + 1) % n].vIdx];
                normal.x += (curr.y - next.y) * (curr.z + next.z);
                normal.y += (curr.z - next.z) * (curr.x + next.x);
                normal.z += (curr.x - next.x) * (curr.y + next.y);
            }

            float len = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (len < 1e-8f) return { 0.0f, 0.0f, 1.0f };

            return { normal.x / len, normal.y / len, normal.z / len };
        };

        auto createMesh = [&]() {
            if (vertexCount == 0) return;

            Mesh mesh = { 0 };
            mesh.vertexCount   = vertexCount;
            mesh.triangleCount = triangleCount;
            mesh.vertices      = (float*)MemAlloc(vertices.size() * sizeof(float));
            mesh.indices       = (unsigned short*)MemAlloc(indices.size() * sizeof(unsigned short));

            memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
            memcpy(mesh.indices,  indices.data(),  indices.size()  * sizeof(unsigned short));

            if (!texcoords.empty()) {
                mesh.texcoords = (float*)MemAlloc(texcoords.size() * sizeof(float));
                memcpy(mesh.texcoords, texcoords.data(), texcoords.size() * sizeof(float));
            }
            if (!normals.empty()) {
                mesh.normals = (float*)MemAlloc(normals.size() * sizeof(float));
                memcpy(mesh.normals, normals.data(), normals.size() * sizeof(float));
            }
            
            UploadMesh(&mesh, false);
            meshes.push_back(mesh);

            vertices.clear();
            texcoords.clear();
            normals.clear();
            indices.clear();
            vertexCount   = 0;
            triangleCount = 0;
        };
        
        for (const auto& faceRef : faces) {
            const auto& face = model.objects[faceRef.objectIdx].faces[faceRef.faceIdx];
            int numVertices = (int)face.vertexData.size();
            if (numVertices < 3) continue;

            if (vertexCount + numVertices > USHRT_MAX) createMesh();

            bool hasNormal = true;
            for (const auto& vertexData : face.vertexData) {
                if (vertexData.vnIdx < 0) { hasNormal = false; break; }
            }

            Vector3 computedNormal{};
            if (!hasNormal) {
                computedNormal = computeFaceNormal(face);
            }

            for (std::size_t i = 0; i < numVertices; ++i) {
                const auto& vertex = model.vertices[face.vertexData[i].vIdx];
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);

                if (!model.textureVertices.empty() && face.vertexData[i].vtIdx >= 0) {
                    const auto& textureVertex = model.textureVertices[face.vertexData[i].vtIdx];
                    // Raylib only allows u,v mapping, ignore w
                    // TODO: Implement custom shader to take in w
                    texcoords.push_back(textureVertex.u);
                    texcoords.push_back(textureVertex.v);
                } else { texcoords.push_back(0.0f); texcoords.push_back(0.0f); }
                if (hasNormal) {
                    const auto& normalVertex = model.normalVertices[face.vertexData[i].vnIdx];
                    normals.push_back(normalVertex.x);
                    normals.push_back(normalVertex.y);
                    normals.push_back(normalVertex.z);
                } else {
                    normals.push_back(computedNormal.x);
                    normals.push_back(computedNormal.y);
                    normals.push_back(computedNormal.z);
                }
            }
            
            unsigned short start = (unsigned short)vertexCount;
            for (std::size_t i = 1; i < numVertices - 1; ++i) {
                indices.push_back(start);
                indices.push_back(start + (unsigned short)(i));
                indices.push_back(start + (unsigned short)(i + 1));
            }

            vertexCount   += numVertices;
            triangleCount += (numVertices - 2);
        }

        createMesh();

        if (meshes.empty()) {
            ::Model empty = { 0 };
            return empty;
        }

        raylibModel.transform = MatrixIdentity();
        raylibModel.meshCount = (int)meshes.size();
        raylibModel.meshes = (::Mesh*)MemAlloc(meshes.size() * sizeof(::Mesh));
        memcpy(raylibModel.meshes, meshes.data(), meshes.size() * sizeof(::Mesh));

        raylibModel.materialCount = 1;
        raylibModel.materials = (::Material*)MemAlloc(sizeof(::Material));
        raylibModel.materials[0] = LoadMaterialDefault();
        raylibModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;

        raylibModel.meshMaterial = (int*)MemAlloc(meshes.size() * sizeof(int));
        for (std::size_t i = 0; i < meshes.size(); ++i) raylibModel.meshMaterial[i] = 0;

        return raylibModel;
    }
} // namespace Morf