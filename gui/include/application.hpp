/* morf - gui/include/application.hpp
 * Application logic to work with external library Raylib
 */
#pragma once
#include "raylib.h"
#include "model.hpp"
#include "diff.hpp"

namespace Morf {
    class Application {
        public:
            Application(int width = 1280, int height = 720);

            bool isMergeMode = false;

            void run(const Model& base, const Model& target, const Diff& diff);

        private:
            int width_, height_;

            Camera3D camera_;
            float dist_    = 5.0f;
            float yaw_     = 0.0f;
            float pitch_   = 0.5f;
            Vector3 target_ = { 0.5f, 0.5f, 0.5f };

            const float rotationalSens = 0.003f;
            const float zoomSpeed      = 0.5f;

            void updateCamera();
    };
} // namespace Morf