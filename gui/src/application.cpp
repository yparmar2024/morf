// morf - gui/src/application.cpp
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "application.hpp"
#include "rlgl.h"
#include "raymath.h"
#include "rcamera.h"
#include <cmath>

namespace Morf {
    Application::Application(int width, int height)
        : width_(width), height_(height)
    {
        camera_.target     = { 0.5f, 0.5f, 0.5f };
        camera_.up         = { 0.0f, 1.0f, 0.0f };
        camera_.fovy       = 45.0f;
        camera_.projection = CAMERA_PERSPECTIVE;

        float dist = 5.0f;
        float yaw = 0.0f;
        float pitch = 0.5f;
        camera_.position = {
            camera_.target.x + dist * cosf(pitch) * cosf(yaw),
            camera_.target.y + dist * sinf(pitch),
            camera_.target.z + dist * cosf(pitch) * sinf(yaw)
        };
    }

    void Application::run(const Model& base, const Model& target, Diff& diff) {
        InitWindow(width_, height_, "morf - Raylib Viewer");
        rlDisableBackfaceCulling();
        SetTargetFPS(60);

        Merge merge = MergeEngine::create(diff);
        Renderer renderer;
        renderer.createModels(base, target, merge);
        renderer.initLighting();

        while (!WindowShouldClose()) {
            updateCamera();
            
            BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera_);

                renderer.drawModels();
                renderer.drawHighlight(base, target, merge);

            EndMode3D();

            DrawText("Left-drag to move\n\nRight-drag to orbit\n\nScroll to zoom", 10, 10, 20, BLACK);
            DrawFPS(10, 100);

            if (isMergeMode) {
                buildMergePanel(base, target, merge, renderer);
            } 
            EndDrawing();
        }
        rlEnableBackfaceCulling();
        renderer.unloadModels();
        CloseWindow();
    }

    void Application::updateCamera() {
        Vector2 delta = GetMouseDelta();
        float wheel   = GetMouseWheelMove();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            float dist = Vector3Distance(camera_.position, camera_.target);
            CameraMoveRight(&camera_, -delta.x * panSensitivity * dist, true);
            CameraMoveUp(&camera_, delta.y * panSensitivity * dist);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            CameraYaw(&camera_, -delta.x * rotationalSens, true);
            CameraPitch(&camera_, -delta.y * rotationalSens, true, true, false);
        }

        if (wheel != 0.0f) {
            CameraMoveToTarget(&camera_, -wheel * zoomSpeed);
            float dist = Vector3Distance(camera_.position, camera_.target);
            if (dist < 0.5f) {
                CameraMoveToTarget(&camera_, dist - 0.5f);
            }
        }
    }

    void Application::buildMergePanel(const Model& base, const Model& target, Merge& merge, Renderer& renderer) {
        const float panelW  = static_cast<float>(width_) / 5.0f;
        const float panelH  = static_cast<float>(height_) / 3.0f;
        const float marginR = (0.02f) * static_cast<float>(width_);
        const float marginT = (0.02f) * static_cast<float>(width_);

        float panelX = static_cast<float>(width_) - panelW - marginR;
        float panelY = marginT;

        const float padX    = (0.05f) * panelW;
        const float padY    = (0.04f) * panelH;
        const float spacing = (0.015f) * panelH;

        const float labelH  = (0.08f) * panelH;
        const float buttonH = (0.09f) * panelH;

        Rectangle panel = { panelX, panelY, panelW, panelH };
        GuiWindowBox(panel, "Merge");

        const float innerX = panelX + padX;
        const float innerW = panelW - 2.0f * padX;
        float currY = panelY + 2.0f * padY;

        if (merge.hasPending()) {
            const bool isAdd = (merge.selectedIndex < merge.added.size());
            const FaceRef& faceRef = isAdd
                ? merge.added[merge.selectedIndex]
                : merge.removed[merge.selectedIndex - merge.added.size()];
            const auto& obj = (isAdd ? target : base).objects[faceRef.objectIdx];
            const auto& face = obj.faces[faceRef.faceIdx];
            const int vertexCount = static_cast<int>(face.vertexData.size());

            const char* changeType = isAdd ? "Added" : "Removed";
            GuiLabel({ innerX, currY, innerW, labelH },
                TextFormat("%s face (%d vertiecs) in '%s'",
                    changeType, vertexCount, obj.name.c_str()));

            currY += labelH + spacing;

            GuiLabel({ innerX, currY, innerW, labelH },
                TextFormat("Change %d / %d",
                    merge.selectedIndex + 1, merge.totalChanges()));

            currY += labelH + spacing;

            const float halfW = (innerW - spacing) / 2.0f;
            if (GuiButton({ innerX, currY, halfW, buttonH }, "Accept")) {
                MergeEngine::accept(merge);
                renderer.createModels(base, target, merge);
                renderer.initLighting();
                focusOnFace(base, target, merge);
            }
            if (GuiButton({ innerX + halfW + spacing, currY, halfW, buttonH }, "Reject")) {
                MergeEngine::reject(merge);
                renderer.createModels(base, target, merge);
                renderer.initLighting();
                focusOnFace(base, target, merge);
            }
            currY += buttonH + spacing;

            if (GuiButton({ innerX, currY, halfW, buttonH }, "Previous")) {
                int total = merge.totalChanges();
                if (total > 0) merge.selectedIndex = (merge.selectedIndex - 1 + total) % total;
                focusOnFace(base, target, merge);
            }
            if (GuiButton({ innerX + halfW + spacing, currY, halfW, buttonH }, "Next")) {
                int total = merge.totalChanges();
                if (total > 0) merge.selectedIndex = (merge.selectedIndex + 1) % total;
                focusOnFace(base, target, merge);
            }
        } else {
            GuiLabel({ innerX, currY, innerW, labelH }, "All changes resolved.");
        }
    }

    void Application::focusOnFace(const Model& base, const Model& target, const Merge& merge) {
        if (!merge.hasPending()) return;

        const Model* srcModel = nullptr;
        FaceRef faceRef;
        if (merge.selectedIndex < merge.added.size()) {
            srcModel = &target;
            faceRef = merge.added[merge.selectedIndex];
        } else {
            srcModel = &base;
            faceRef = merge.removed[merge.selectedIndex - merge.added.size()];
        }

        const auto& obj = srcModel->objects[faceRef.objectIdx];
        const auto& face = obj.faces[faceRef.faceIdx];
        int n = static_cast<int>(face.vertexData.size());

        Vector3 center = { 0, 0, 0 };
        for (int i = 0; i < n; ++i) {
            const auto& vertex = srcModel->vertices[face.vertexData[i].vIdx];
            center.x += vertex.x;
            center.y += vertex.y;
            center.z += vertex.z;
        }
        float invN = 1.0f / n;
        center.x *= invN;
        center.y *= invN;
        center.z *= invN;

        Vector3 normal = computeFaceNormal(*srcModel, face);

        normal.z = -normal.z;
        float pitch = asinf(normal.y);
        float yaw   = atan2f(normal.z, normal.x);

        if (pitch > 1.5f)  pitch = 1.5f;
        if (pitch < -1.5f) pitch = -1.5f;

        Vector3 safeNormal = {
            cosf(pitch) * cosf(yaw),
            sinf(pitch),
            cosf(pitch) * sinf(yaw)
        };
        
        float dist = Vector3Distance(camera_.position, camera_.target);
        if (dist < 0.5f) dist = 5.0f;

        camera_.position = {
            center.x + safeNormal.x * dist,
            center.y + safeNormal.y * dist,
            center.z + safeNormal.z * dist
        };
        camera_.target = center;
    }
} // namespace Morf