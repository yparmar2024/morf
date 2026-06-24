/* morf - gui/include/renderer.hpp
 * Rendering logic to convert internal model to Raylib-facing model
 */
#pragma once
#include "raylib.h"
#include "model.hpp"
#include "merge.hpp"
#include <vector>

namespace Morf {
    Vector3 computeFaceNormal(const Model& model, const Face& face);

    class Renderer {
        public:
            void createModels(const Model& base, const Model& target, const Merge& merge);
            void initLighting();
            void drawModels() const;
            void drawHighlight(const Model& base, const Model& target, const Merge& merge) const;
            void unloadModels();
        
        private:
            ::Model commonModel_     = { 0 };
            ::Model keptBaseModel_   = { 0 };
            ::Model keptTargetModel_ = { 0 };
            ::Model addedModel_      = { 0 };
            ::Model removedModel_    = { 0 };

            bool loaded_ = false;
            
            ::Model buildRaylibModel(const Model& model, const std::vector<FaceRef>& faces, Color color) const;
    };
} // namespace Morf