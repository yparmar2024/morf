// morf - gui/src/application.cpp
#include "application.hpp"
#include "renderer.hpp"
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

    void Application::run(const Model& base, const Model& target, const Diff& diff) {
        InitWindow(width_, height_, "morf - Raylib Viewer");
        SetTargetFPS(60);

        Renderer renderer;
        renderer.createModels(base, target, diff);

        while(!WindowShouldClose()) {
            updateCamera();
            
            BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera_);

                renderer.drawModels();

            EndMode3D();

            if (isMergeMode) {
                // TODO: Add merge ImGui UI
                continue;
            } else {
                DrawText("Right-drag to orbit | Scroll to zoom", 10, 10, 20, LIGHTGRAY);
                DrawFPS(10, 40);
            }

            EndDrawing();
        }
        renderer.unloadModels();
        CloseWindow();
    }

    void Application::updateCamera() {
        Vector2 delta = GetMouseDelta();
        float wheel   = GetMouseWheelMove();

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            yaw_   -= delta.x * rotationalSens;
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
} // namespace Morf