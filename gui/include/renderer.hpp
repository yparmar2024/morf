/* morf - gui/include/renderer.hpp
 * Rendering logic to convert internal model to Raylib-facing model
 */
#pragma once
#include "raylib.h"
#include "model.hpp"
#include "diff.hpp"
#include <vector>

namespace Morf {
    class Renderer {
        public:
            void createModels(const Model& base, const Model& target, const Diff& diff);
            void initLighting();
            void drawModels() const;
            void unloadModels();
        
        private:
            std::vector<::Model> commonModels_{0};
            std::vector<::Model> addedModels_{0};
            std::vector<::Model> removedModels_{0};

            bool loaded_ = false;
            
            std::vector<::Model> buildRaylibModels(const Model& model, const std::vector<FaceRef>& faces, Color color) const;
    };
} // namespace Morf