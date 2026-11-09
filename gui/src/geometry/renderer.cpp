// morf - gui/src/geometry/renderer.cpp
#include "application/session.hpp"
#include "geometry/renderer.hpp"
#include "raymath.h"
#include "tesselator.h"

#include <climits>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <unordered_map>

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

    void Renderer::createModels(const Session& session) {
        if (loaded_) unloadModels();

        loaded_ = true;
        try {
            baseCommonModel_ = buildModel(session.getBaseVertices(), session.getCommonBaseIndices(), LIGHTGRAY);
            ourCommonModel_ = buildModel(session.getOurVertices(), session.getOurBaseIndices(), LIGHTGRAY);
            theirCommonModel_ = buildModel(session.getTheirVertices(), session.getTheirBaseIndices(), LIGHTGRAY);

            ourAddsModel_ = buildModel(session.getOurVertices(), session.getOurAddedIndices(), {0, 255, 0, 180});
            ourRemovesModel_ = buildModel(session.getBaseVertices(), session.getOurRemovedIndices(), {255, 0, 0, 180});
            theirAddsModel_ = buildModel(session.getTheirVertices(), session.getTheirAddedIndices(), {0, 255, 0, 180});
            theirRemovesModel_ =
                buildModel(session.getBaseVertices(), session.getTheirRemovedIndices(), {255, 0, 0, 180});

            initLighting();
        } catch (...) {
            unloadModels();
            throw;
        }
    }

    ::Model Renderer::buildModel(const std::vector<std::array<float, 3>>& vertices,
                                 const std::vector<std::vector<std::size_t>>& indices, Color color) {
        if (indices.empty()) return {0};
        for (const auto& face : indices) {
            if (face.size() < 3 || face.size() > static_cast<std::size_t>(INT_MAX / 3))
                throw std::runtime_error("Invalid face size for rendering.");
            for (auto index : face)
                if (index >= vertices.size()) throw std::out_of_range("Render face index out of range.");
        }

        ::Model model = {0};
        std::vector<Mesh> meshes;
        struct MeshCleanup {
                std::vector<Mesh>& meshes;
                bool transferred = false;
                explicit MeshCleanup(std::vector<Mesh>& meshes) : meshes(meshes) {}
                ~MeshCleanup() {
                    if (!transferred)
                        for (auto mesh : meshes) UnloadMesh(mesh);
                }
        } cleanup{meshes};

        // Temporary mesh accumulators
        std::vector<float> meshVertices;
        std::vector<float> meshTexcoords;
        std::vector<float> meshNormals;
        std::vector<unsigned short> meshIndices;

        struct Vector3Hash {
                std::size_t operator()(const std::array<Vector3, 2>& key) const {
                    std::size_t h1 = std::hash<float>{}(key[0].x) ^ (std::hash<float>{}(key[1].x) << 1);
                    std::size_t h2 = std::hash<float>{}(key[0].y) ^ (std::hash<float>{}(key[1].y) << 1);
                    std::size_t h3 = std::hash<float>{}(key[0].z) ^ (std::hash<float>{}(key[1].z) << 1);
                    return h1 ^ (h2 << 1) ^ (h3 << 2);
                }
        };
        struct Vector3Equal {
                bool operator()(const std::array<Vector3, 2>& a, const std::array<Vector3, 2>& b) const {
                    return a[0].x == b[0].x && a[0].y == b[0].y && a[0].z == b[0].z && a[1].x == b[1].x &&
                           a[1].y == b[1].y && a[1].z == b[1].z;
                }
        };

        std::unordered_map<std::array<Vector3, 2>, unsigned short, Vector3Hash, Vector3Equal> vertexMap;

        auto createMesh = [&]() -> void {
            if (meshIndices.empty()) return;

            Mesh mesh = {0};
            mesh.vertexCount = static_cast<int>(meshVertices.size() / 3);
            mesh.triangleCount = static_cast<int>(meshIndices.size() / 3);
            mesh.vertices = (float*)MemAlloc(meshVertices.size() * sizeof(float));
            mesh.indices = (unsigned short*)MemAlloc(meshIndices.size() * sizeof(unsigned short));
            mesh.normals = (float*)MemAlloc(meshNormals.size() * sizeof(float));

            if (!mesh.vertices || !mesh.indices || !mesh.normals) {
                MemFree(mesh.vertices);
                MemFree(mesh.indices);
                MemFree(mesh.normals);
                throw std::runtime_error("Failed to allocate render mesh.");
            }
            memcpy(mesh.vertices, meshVertices.data(), meshVertices.size() * sizeof(float));
            memcpy(mesh.indices, meshIndices.data(), meshIndices.size() * sizeof(unsigned short));
            memcpy(mesh.normals, meshNormals.data(), meshNormals.size() * sizeof(float));

            if (!meshTexcoords.empty()) {
                mesh.texcoords = (float*)MemAlloc(meshTexcoords.size() * sizeof(float));
                memcpy(mesh.texcoords, meshTexcoords.data(), meshTexcoords.size() * sizeof(float));
            }

            UploadMesh(&mesh, false);
            meshes.push_back(mesh);

            meshVertices.clear();
            meshIndices.clear();
            meshNormals.clear();
            meshTexcoords.clear();
            vertexMap.clear();
        };

        std::vector<float> contourVertices;
        std::unique_ptr<TESStesselator, decltype(&tessDeleteTess)> tessOwner(tessNewTess(nullptr), tessDeleteTess);
        auto* tess = tessOwner.get();
        if (!tess) throw std::runtime_error("Failed to allocate tessellator.");
        for (const auto& contour : indices) {
            int numVertices = static_cast<int>(contour.size());

            contourVertices.clear();
            contourVertices.reserve(numVertices * 3);

            for (std::size_t idx : contour) {
                contourVertices.push_back(vertices[idx][0]);
                contourVertices.push_back(vertices[idx][1]);
                contourVertices.push_back(vertices[idx][2]);
            }

            tessAddContour(tess, 3, contourVertices.data(), 3 * sizeof(float), numVertices);
            if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 3, nullptr))
                throw std::runtime_error("Failed to tessellate face.");

            const float* tessVertices = tessGetVertices(tess);
            const int* tessIndices = tessGetElements(tess);
            int totalVertices = tessGetVertexCount(tess);
            int totalTriangles = tessGetElementCount(tess);

            if (totalTriangles < 0 || totalVertices < 0 || (totalTriangles > 0 && (!tessVertices || !tessIndices)))
                throw std::runtime_error("Invalid tessellation output.");
            for (std::size_t k = 0; k < 3 * static_cast<std::size_t>(totalTriangles); ++k)
                if (tessIndices[k] < 0 || tessIndices[k] >= totalVertices)
                    throw std::runtime_error("Invalid tessellation vertex index.");

            Vector3 computedNormal = {0};
            for (std::size_t i = 0; i < 3 * static_cast<std::size_t>(totalTriangles); ++i) {
                // Flush only between triangles; a single large polygon can exceed 16-bit indices.
                if (i % 3 == 0 && meshVertices.size() / 3 + 3 > USHRT_MAX) createMesh();
                int idx = tessIndices[i];
                Vector3 v = {tessVertices[(3 * idx)], tessVertices[(3 * idx) + 1], tessVertices[(3 * idx) + 2]};

                // If it's the first index of a triangle, fetch the other two vertices to compute a normal
                // for our shader, and re-use that normal for the remaining indices, repeat for each triangle
                if (i % 3 == 0) {
                    int idx1 = tessIndices[i + 1];
                    int idx2 = tessIndices[i + 2];
                    Vector3 v1 = {tessVertices[3 * (idx1)], tessVertices[(3 * idx1) + 1], tessVertices[(3 * idx1) + 2]};
                    Vector3 v2 = {tessVertices[(3 * idx2)], tessVertices[(3 * idx2) + 1], tessVertices[(3 * idx2) + 2]};
                    computedNormal =
                        Vector3Normalize(Vector3CrossProduct(Vector3Subtract(v1, v), Vector3Subtract(v2, v)));
                }

                std::array<Vector3, 2> key = {v, computedNormal};
                if (vertexMap.find(key) != vertexMap.end()) {
                    meshIndices.push_back(vertexMap[key]);
                } else {
                    vertexMap[key] = meshVertices.size() / 3;
                    meshIndices.push_back(meshVertices.size() / 3);

                    meshVertices.push_back(v.x);
                    meshVertices.push_back(v.y);
                    meshVertices.push_back(v.z);

                    meshNormals.push_back(computedNormal.x);
                    meshNormals.push_back(computedNormal.y);
                    meshNormals.push_back(computedNormal.z);
                }
            }
        }
        tessOwner.reset();
        createMesh();

        if (meshes.empty()) return {0};

        model.transform = MatrixIdentity();
        model.meshCount = (int)meshes.size();
        model.meshes = (::Mesh*)MemAlloc(meshes.size() * sizeof(::Mesh));
        model.materialCount = 1;
        model.materials = (::Material*)MemAlloc(sizeof(::Material));
        model.meshMaterial = (int*)MemAlloc(meshes.size() * sizeof(int));
        if (!model.meshes || !model.materials || !model.meshMaterial) {
            MemFree(model.meshes);
            MemFree(model.materials);
            MemFree(model.meshMaterial);
            throw std::runtime_error("Failed to allocate render model.");
        }
        memcpy(model.meshes, meshes.data(), meshes.size() * sizeof(::Mesh));
        model.materials[0] = LoadMaterialDefault();
        if (!model.materials[0].maps) {
            MemFree(model.meshes);
            MemFree(model.materials);
            MemFree(model.meshMaterial);
            throw std::runtime_error("Failed to allocate render material.");
        }
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;

        for (std::size_t i = 0; i < meshes.size(); ++i) model.meshMaterial[i] = 0;

        cleanup.transferred = true;
        return model;
    }

    void Renderer::drawFullView() {
        if (!loaded_) return;

        DrawModel(baseCommonModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(ourCommonModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(theirCommonModel_, {0, 0, 0}, 1.0f, WHITE);

        BeginBlendMode(BLEND_ALPHA);
        DrawModel(ourAddsModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(ourRemovesModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(theirAddsModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(theirRemovesModel_, {0, 0, 0}, 1.0f, WHITE);
        EndBlendMode();
    }

    void Renderer::drawLeftView() {
        if (!loaded_) return;

        DrawModel(baseCommonModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(ourCommonModel_, {0, 0, 0}, 1.0f, WHITE);

        BeginBlendMode(BLEND_ALPHA);
        DrawModel(ourAddsModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(ourRemovesModel_, {0, 0, 0}, 1.0f, WHITE);
        EndBlendMode();
    }

    void Renderer::drawRightView() {
        if (!loaded_) return;

        DrawModel(baseCommonModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(theirCommonModel_, {0, 0, 0}, 1.0f, WHITE);

        BeginBlendMode(BLEND_ALPHA);
        DrawModel(theirAddsModel_, {0, 0, 0}, 1.0f, WHITE);
        DrawModel(theirRemovesModel_, {0, 0, 0}, 1.0f, WHITE);
        EndBlendMode();
    }

    void Renderer::drawHighlight(const Session& session) {
        if (!loaded_ || session.getTotalConflicts() == 0) return;

        int idx = session.getSelectedIndex();
        const std::vector<std::size_t>* face = nullptr;
        const std::vector<std::array<float, 3>>* vertices = nullptr;

        if (idx < session.getOurAddedIndices().size()) {
            face = &session.getOurAddedIndices()[idx];
            vertices = &session.getOurVertices();
        } else if ((idx -= session.getOurAddedIndices().size()) < session.getOurRemovedIndices().size()) {
            face = &session.getOurRemovedIndices()[idx];
            vertices = &session.getBaseVertices();
        } else if ((idx -= session.getOurRemovedIndices().size()) < session.getTheirAddedIndices().size()) {
            face = &session.getTheirAddedIndices()[idx];
            vertices = &session.getTheirVertices();
        } else {
            idx -= session.getTheirAddedIndices().size();
            face = &session.getTheirRemovedIndices()[idx];
            vertices = &session.getBaseVertices();
        }

        std::vector<Vector3> points;
        points.reserve(face->size());
        for (std::size_t vIdx : *face) {
            points.push_back({(*vertices)[vIdx][0], (*vertices)[vIdx][1], (*vertices)[vIdx][2]});
        }
        if (points.size() < 3) return;

        float pulse = (sinf(GetTime() * 6.0f) + 1.0f) / 2.0f;

        unsigned char fillAlpha = (unsigned char)(50 + pulse * 100);
        Color highlightColor = {255, 255, 255, fillAlpha};
        for (std::size_t i = 1; i < points.size() - 1; ++i)
            DrawTriangle3D(points[0], points[i], points[i + 1], highlightColor);

        unsigned char edgeAlpha = (unsigned char)(100 + pulse * 155);
        Color edgeColor = {0, 0, 0, edgeAlpha};
        for (std::size_t i = 0; i < points.size(); ++i)
            DrawLine3D(points[i], points[(i + 1) % points.size()], edgeColor);
    }

    void Renderer::unloadModels() {
        if (!loaded_) return;

        UnloadModel(baseCommonModel_);
        UnloadModel(ourCommonModel_);
        UnloadModel(theirCommonModel_);

        UnloadModel(ourAddsModel_);
        UnloadModel(ourRemovesModel_);
        UnloadModel(theirAddsModel_);
        UnloadModel(theirRemovesModel_);

        baseCommonModel_ = {0};
        ourCommonModel_ = {0};
        theirCommonModel_ = {0};

        ourAddsModel_ = {0};
        ourRemovesModel_ = {0};
        theirAddsModel_ = {0};
        theirRemovesModel_ = {0};

        loaded_ = false;
    }

    void Renderer::initLighting() {
        static Shader lightingShader = []() {
            Shader shader = LoadShaderFromMemory(lightingVs, lightingFs);

            float ambient[4] = {0.5f, 0.5f, 0.5f, 1.0f};
            float lightDir[3] = {0.5f, 1.0f, 0.8f};
            float lightCol[4] = {0.9f, 0.9f, 0.9f, 1.0f};

            SetShaderValue(shader, GetShaderLocation(shader, "ambient"), ambient, SHADER_UNIFORM_VEC4);
            SetShaderValue(shader, GetShaderLocation(shader, "lightPos"), lightDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, GetShaderLocation(shader, "lightColor"), lightCol, SHADER_UNIFORM_VEC4);

            return shader;
        }();

        auto applyShader = [&](::Model& model) {
            for (std::size_t i = 0; i < model.materialCount; ++i) {
                model.materials[i].shader = lightingShader;
            }
        };

        applyShader(baseCommonModel_);
        applyShader(ourCommonModel_);
        applyShader(theirCommonModel_);
    }
} // namespace Morf
