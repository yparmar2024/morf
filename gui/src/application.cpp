// morf - gui/src/application.cpp
#include "application.hpp"
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <cmath>

namespace Morf {
    Application::Application(int width, int height)
        : width_(width), height_(height)
    {
        camera_.position   = { 0.0f, 0.0f, 0.0f };
        camera_.target     = target_;
        camera_.up         = { 0.0f, 1.0f, 0.0f };
        camera_.fovy       = 45.0f;
        camera_.projection = CAMERA_PERSPECTIVE;
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
            ClearBackground(BLACK);
            BeginMode3D(camera_);

                renderer.drawModels();
                renderer.drawHighlight(base, target, merge);

            EndMode3D();

            DrawText("Left-drag to move\n\nRight-drag to orbit\n\nScroll to zoom", 10, 10, 20, LIGHTGRAY);
            DrawFPS(10, 100);

            if (isMergeMode) {
                buildMergePanel(base, target, merge, renderer);
            } 
            EndDrawing();
        }
        renderer.unloadModels();
        CloseWindow();
    }

    void Application::updateCamera() {
        Vector2 delta = GetMouseDelta();
        float wheel   = GetMouseWheelMove();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector3 forward = { camera_.target.x - camera_.position.x,
                                camera_.target.y - camera_.position.y,
                                camera_.target.z - camera_.position.z };
            float len = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
            forward.x /= len; forward.y /= len; forward.z /= len;

            Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
            Vector3 right = { forward.y * worldUp.z - forward.z * worldUp.y,
                              forward.z * worldUp.x - forward.x * worldUp.z,
                              forward.x * worldUp.y - forward.y * worldUp.x };
            Vector3 up = { right.y * forward.z - right.z * forward.y,
                           right.z * forward.x - right.x * forward.z,
                           right.x * forward.y - right.y * forward.x };

            float panScale = dist_ * panSensitivity;
            Vector3 translation = { -delta.x * panScale * right.x + delta.y * panScale * up.x,
                                    -delta.x * panScale * right.y + delta.y * panScale * up.y,
                                    -delta.x * panScale * right.z + delta.y * panScale * up.z };
            target_.x += translation.x;
            target_.y += translation.y;
            target_.z += translation.z;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            yaw_   += delta.x * rotationalSens;
            pitch_ += delta.y * rotationalSens;
            
            if (pitch_ > 1.5f)  pitch_ = 1.5f;
            if (pitch_ < -1.5f) pitch_ = -1.5f;
        }

        dist_ -= wheel * zoomSpeed;
        if (dist_ < 0.5f) dist_ = 0.5f;

        float x = target_.x + dist_ * cosf(pitch_) * cosf(yaw_);
        float y = target_.y + dist_ * sinf(pitch_);
        float z = target_.z + dist_ * cosf(pitch_) * sinf(yaw_);
        camera_.position = { x, y, z };
        camera_.target   = target_;
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
        int n = (int)face.vertexData.size();

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

        target_ = center;
    }
} // namespace Morf