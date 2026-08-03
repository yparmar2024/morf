// morf - gui/src/application/application.cpp
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "application/application.hpp"
#include "application/session.hpp"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"

namespace Morf {
    Application::Application() {
        camera_.target     = { 0.5f, 0.5f, 0.5f };
        camera_.up         = { 0.0f, 1.0f, 0.0f };
        camera_.fovy       = 45.0f;
        camera_.projection = CAMERA_PERSPECTIVE;

        float dist  = 5.0f;
        float yaw   = 0.0f;
        float pitch = 0.5f;
        camera_.position = {
            camera_.target.x + dist * cosf(pitch) * cosf(yaw),
            camera_.target.y + dist * sinf(pitch),
            camera_.target.z + dist * cosf(pitch) * sinf(yaw)
        };
    }
    
    void Application::run(Session& session) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
        InitWindow(0, 0, "morf - Raylib Viewer");
        SetWindowMinSize(GetMonitorWidth(0) * 0.50f, GetMonitorHeight(0) * 0.50f);
        rlDisableBackfaceCulling();
        SetTargetFPS(60);

        Renderer renderer;
        renderer.createModels(session);

        focusOnFace(session);

        while (!WindowShouldClose()) {
            updateCamera();

            BeginDrawing();

                ClearBackground(RAYWHITE);

                if (session.isResolved()) {
                    BeginMode3D(camera_);

                        renderer.drawFullView();
                        
                    EndMode3D();

                    const float btnWidth = 200.0f;
                    const float btnHeight = 40.0f;
                    const float panelW = btnWidth + 20.0f;
                    const float panelH = btnHeight + 40.0f;

                    float panelX = (GetScreenWidth() - panelW) / 2.0f;
                    float panelY = GetScreenHeight() - panelH - 30.0f;
                    const char* btnText = (session.getMode() == "diff") ? "Exit" : "Save & Exit";

                    if (GuiButton({ panelX + 10.0f, panelY + 30.0f, btnWidth, btnHeight}, btnText)) {
                        break;
                    }
                } else {
                    rlViewport(0, 0, GetScreenWidth() / 2, GetScreenHeight());
                    BeginMode3D(camera_);

                        rlMatrixMode(RL_PROJECTION);
                        rlLoadIdentity();

                        double aspect = (double)(GetScreenWidth() / 2.0f) / (double)GetScreenHeight();
                        double top = 0.01 * tan(camera_.fovy * 0.5 * DEG2RAD);
                        double right = top * aspect;

                        rlFrustum(-right, right, -top, top, 0.01, 1000.0);
                        rlMatrixMode(RL_MODELVIEW);

                        renderer.drawLeftView();
                        renderer.drawHighlight(session);

                    EndMode3D();

                    rlViewport(GetScreenWidth() / 2, 0, GetScreenWidth() / 2, GetScreenHeight());
                    BeginMode3D(camera_);

                        rlMatrixMode(RL_PROJECTION);
                        rlLoadIdentity();

                        rlFrustum(-right, right, -top, top, 0.01, 1000.0);
                        rlMatrixMode(RL_MODELVIEW);

                        renderer.drawRightView();
                        renderer.drawHighlight(session);
                        
                    EndMode3D();

                    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

                    DrawText("Our Changes", (GetScreenWidth() / 4) - (MeasureText("Our Changes", 30) / 2), GetScreenHeight() - 50, 30, BLACK);
                    DrawText("Their Changes", (3 * GetScreenWidth() / 4) - (MeasureText("Their Changes", 30) / 2), GetScreenHeight() - 50, 30, BLACK);
                    DrawRectangle(GetScreenWidth() / 2 - 2, 0, 4, GetScreenHeight(), BLACK);
                    buildMergePanel(session, renderer);
                }

            DrawText("Left-drag to move\n\nRight-drag to orbit\n\nScroll to zoom", 10, 10, 20, BLACK);
            DrawFPS(10, 100);

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
            CameraMoveRight(&camera_, -delta.x * panSens * dist, true);
            CameraMoveUp(&camera_, delta.y * panSens * dist);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            CameraYaw(&camera_, -delta.x * rotSens, true);
            CameraPitch(&camera_, -delta.y * rotSens, true, true, false);
        }

        if (wheel != 0.0f) {
            CameraMoveToTarget(&camera_, -wheel * zoomSpeed);
            float dist = Vector3Distance(camera_.position, camera_.target);
            if (dist < 0.5f) {
                CameraMoveToTarget(&camera_, dist - 0.5f);
            }
        }
    }
    
    void Application::focusOnFace(const Session& session) {
        if (session.getTotalConflicts() == 0) return;

        int idx = session.getSelectedIndex();
        const std::vector<std::size_t>* face = nullptr;
        const std::vector<std::array<float, 3>>* vertices = nullptr;

        if (idx < session.getOurAddedIndices().size()) {
            face     = &session.getOurAddedIndices()[idx];
            vertices = &session.getOurVertices();
        } else if ((idx -= session.getOurAddedIndices().size()) < session.getOurRemovedIndices().size()) {
            face     = &session.getOurRemovedIndices()[idx];
            vertices = &session.getBaseVertices();
        } else if ((idx -= session.getOurRemovedIndices().size()) < session.getTheirAddedIndices().size()) {
            face     = &session.getTheirAddedIndices()[idx];
            vertices = &session.getTheirVertices();
        } else {
            idx     -= session.getTheirAddedIndices().size();
            face     = &session.getTheirRemovedIndices()[idx];
            vertices = &session.getBaseVertices();
        }

        Vector3 center = { 0, 0, 0 };
        for (std::size_t vIdx : *face) {
            center.x += (*vertices)[vIdx][0];
            center.y += (*vertices)[vIdx][1];
            center.z += (*vertices)[vIdx][2];
        }
        float invN = 1.0f / face->size();
        center.x *= invN;
        center.y *= invN;
        center.z *= invN;

        Vector3 normal = { 0, 0, 0 };
        const auto& v0 = (*vertices)[(*face)[0]];
        for (std::size_t k = 1; k < face->size() - 1; ++k) {
            const auto& v1 = (*vertices)[(*face)[k]];
            const auto& v2 = (*vertices)[(*face)[k + 1]];

            float e1x = v1[0] - v0[0], e1y = v1[1] - v0[1], e1z = v1[2] - v0[2];
            float e2x = v2[0] - v0[0], e2y = v2[1] - v0[1], e2z = v2[2] - v0[2];

            normal.x += (e1y * e2z - e1z * e2y);
            normal.y += (e1z * e2x - e1x * e2z);
            normal.z += (e1x * e2y - e1y * e2x);
        }

        float len = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (len > 0.0f) {
            normal.x /= len;
            normal.y /= len;
            normal.z /= len;
        } else {
            normal = { 0, 1, 0 };
        }

        normal.z = -normal.z;
        float pitch = asinf(normal.y);
        float yaw   = atan2(normal.z, normal.x);

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

    void Application::buildMergePanel(Session& session, Renderer& renderer) {
        const float width  = GetScreenWidth();
        const float height = GetScreenHeight();

        int responsiveFontSize = std::max(10, (int)(height * 0.01f));
        GuiSetStyle(DEFAULT, TEXT_SIZE, responsiveFontSize);

        const float panelW = std::max(220.0f, width / 10.0f);

        const float labelH = std::max(20.0f, 0.02f * height);
        const float buttonH = std::max(30.0f, 0.03f * height);
        const float spacing = std::max(8.0f, 0.01f * height);
        const float padX = (0.05f) * panelW;
        const float padY = std::max(15.0f, 0.02f * height);

        const float panelH = session.isResolved()
            ? (30.0f + labelH + padY)
            : (30.0f + labelH * 2 + spacing * 3 + buttonH * 2 + padY);

        const float marginR = (0.01f) * width;
        const float marginT = (0.01f) * height;

        float panelX = width - panelW - marginR;
        float panelY = marginT;

        Rectangle panel = { panelX, panelY, panelW, panelH };
        GuiWindowBox(panel, "Merge");

        const float innerX = panelX + padX;
        const float innerW = panelW - 2.0f * padX;
        float currY = panelY + 2.0f * padY;

        if (session.isResolved()) {
            GuiLabel({ innerX, currY, innerW, labelH }, "All conflicts resolved.");
            return;
        }

        GuiLabel({ innerX, currY, innerW, labelH },
            TextFormat("Conflict %d / %d", session.getSelectedIndex() + 1, session.getTotalConflicts())
        );
        currY += labelH + spacing;

        const float halfW = (innerW - spacing) / 2.0f;
        if (GuiButton({ innerX, currY, halfW, buttonH }, "Accept Ours")) {
            session.acceptOurs();
            renderer.createModels(session);
        }
        if (GuiButton({ innerX + halfW + spacing, currY, halfW, buttonH }, "Accept Theirs")) {
            session.acceptTheirs();
            renderer.createModels(session);
        }
        currY += buttonH + spacing;

        if (GuiButton({ innerX, currY, halfW, buttonH }, "Previous")) {
            session.prevConflict();
            focusOnFace(session);
        }
        if (GuiButton({ innerX + halfW + spacing, currY, halfW, buttonH }, "Next")) {
            session.nextConflict();
            focusOnFace(session);
        }
    }
} // namespace Morf