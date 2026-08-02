/* morf - gui/include/geometry/renderer.hpp
 * Rendering logic to convert internal model to Raylib-facing model
 */
#pragma once
#include "raylib.h"
#include "application/session.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace Morf {
    class Renderer {
        private:
            ::Model baseCommonModel_   = { 0 };
            ::Model ourCommonModel_    = { 0 };
            ::Model theirCommonModel_  = { 0 };

            ::Model ourAddsModel_      = { 0 };
            ::Model ourRemovesModel_   = { 0 };
            ::Model theirAddsModel_    = { 0 };
            ::Model theirRemovesModel_ = { 0 };

            bool loaded_ = false;
        
        public:
            void createModels(const Session& session);
            ::Model buildModel(
                const std::vector<std::array<float, 3>>& vertices,
                const std::vector<std::vector<std::size_t>>& indices,
                Color color
            );
            void drawFullView();
            void drawLeftView();
            void drawRightView();
            void drawHighlight(const Session& session);
            void unloadModels();
            void initLighting();

    };
} // namespace Morf