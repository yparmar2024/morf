#pragma once
#include "raylib.h"
#include "model.hpp"
#include "diff.hpp"

namespace Morf {
    /* Application object to run application */
    class Application {
        public:
            // Constructor for application
            Application(int width = 1280, int height = 720);

            // Boolean for merge mode
            bool isMergeMode = false;

            // Run application with models and diff
            void run(const Model& base, const Model& target, const Diff& diff);

        private:
            // Window dimensions
            int width_, height_;

            // Camera attributes
            Camera3D camera_;
            float dist_    = 5.0f;
            float yaw_     = 0.0f;
            float pitch_   = 0.5f;
            Vector3 target_ = { 0.5f, 0.5f, 0.5f };

            const float rotationalSens = 0.003f;
            const float zoomSpeed      = 0.5f;

            // Update the camera
            void updateCamera();
    };
}