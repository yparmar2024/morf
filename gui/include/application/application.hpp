/* morf - gui/include/application/application.hpp
 * Application logic to work with external library Raylib
 */
#pragma once
#include "raylib.h"
#include "application/session.hpp"
#include "geometry/renderer.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace Morf {
    class Application {
        private:
            Camera3D camera_;

            const float rotSens   = 0.003f;
            const float zoomSpeed = 0.5f;
            const float panSens   = 0.003f;

            void updateCamera();
            void focusOnFace(const Session& session);
            void buildMergePanel(Session& session, Renderer& renderer);

        public:
            Application();
            void run(Session& session);
    };
} // namespace Morf