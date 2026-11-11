/* morf - gui/include/morf/viewer/viewer.hpp
 * Raylib window, camera controls, and merge panel.
 */
#pragma once
#include "morf/merge/comparison_session.hpp"
#include "morf/viewer/mesh_renderer.hpp"
#include "raylib.h"

namespace Morf {
    class Viewer {
        private:
            Camera3D camera_;

            const float rotSens = 0.003f;
            const float zoomSpeed = 0.5f;
            const float panSens = 0.003f;

            void updateCamera();
            void focusOnFace(const ComparisonSession& session);
            void buildMergePanel(ComparisonSession& session, MeshRenderer& renderer);

        public:
            Viewer();
            // True only when the user explicitly chooses to save a resolved merge.
            bool run(ComparisonSession& session);
    };
} // namespace Morf
