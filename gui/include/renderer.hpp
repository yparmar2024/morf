#pragma once
#include "raylib.h"
#include "model.hpp"
#include "diff.hpp"
#include <vector>

namespace Morf {
    /* Renderer object to render meshes */
    class Renderer {
        public:
            // Create meshes from models and diff
            void createMeshes(const Model& base, const Model& target, const Diff& diff);

            // Draw meshes in window
            void drawMeshes() const;

            // Unload resources from meshes
            void unloadMeshes();
        
        private:
            // Build a mesh
            Mesh buildMesh(const Model& model, const std::vector<FaceRef>& faces) const;

            // Raylib meshes
            Mesh commonMesh_{0};
            Mesh addedMesh_{0};
            Mesh removedMesh_{0};

            // Raylib materials
            Material greyMat_{0};
            Material greenMat_{0};
            Material redMat_{0};

            // Loaded boolean
            bool loaded_ = false;
    };
}