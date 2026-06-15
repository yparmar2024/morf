#include "application.hpp"
#include "renderer.hpp"
#include <cmath>

namespace Morf {
    /* Constructor to initialize application */
    Application::Application(int width, int height)
        : width_(width), height_(height)
    {
        // Initialize camera attributes
        camera_.position   = { 0.0f, 0.0f, 0.0f };
        camera_.target     = target_;
        camera_.up         = { 0.0f, 1.0f, 0.0f };
        camera_.fovy       = 45.0f;
        camera_.projection = CAMERA_PERSPECTIVE;
    }

    /* Method to run application */
    void Application::run(const Model& base, const Model& target, const Diff& diff) {
        // Initialize window
        InitWindow(width_, height_, "morf - Raylib Viewer");
        SetTargetFPS(60);

        // Initialize renderer and load the meshes
        Renderer renderer;
        renderer.createMeshes(base, target, diff);

        // While window is open, continuously update the camera
        while(!WindowShouldClose()) {
            updateCamera();
            
            BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera_);

                renderer.drawMeshes();

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
        
        // Unload resources and close the window
        renderer.unloadMeshes();
        CloseWindow();
    }

    /* Method to update camera */
    void Application::updateCamera() {
        // Fetch camera and zoom movements
        Vector2 delta = GetMouseDelta();
        float wheel   = GetMouseWheelMove();

        // If right dragging, perform orbit logic
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            // Compute new yaw and pitch
            yaw_   -= delta.x * rotationalSens;
            pitch_ += delta.y * rotationalSens;
            
            // Reset pitch if above or below bounds
            if (pitch_ > 1.5f)  pitch_ = 1.5f;
            if (pitch_ < -1.5f) pitch_ = -1.5f;
        }

        // If scrolling, perform zoom logic
        dist_ -= wheel * zoomSpeed;
        if (dist_ < 0.5f) dist_ = 0.5f;

        // Compute coordinates and update
        float x = target_.x + dist_ * cosf(pitch_) * cosf(yaw_);
        float y = target_.y + dist_ * sinf(pitch_);
        float z = target_.z + dist_ * cosf(pitch_) * sinf(yaw_);
        camera_.position = { x, y, z };
        camera_.target   = target_;
    }
}