// morf - cli/src/command/merge.cpp
#include "command/load_models.hpp"
#include "command/merge.hpp"
#include "morf/compare/mesh_diff.hpp"
#include "morf/formats/obj/obj_io.hpp"
#include "morf/geometry/mesh.hpp"
#include "morf/merge/comparison_session.hpp"
#include "morf/viewer/viewer.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <vector>

namespace Morf {
    static std::string getExtension(const std::string& filePath) {
        std::string ext = std::filesystem::path(filePath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    int Merge::run(const std::string& baseFilePath, const std::string& ourFilePath, const std::string& theirFilePath,
                   const std::string& outputFilePath, bool benchmark) {
        if (!std::filesystem::exists(baseFilePath)) {
            throw std::runtime_error("Base file '" + baseFilePath + "' does not exist.");
        }
        if (!std::filesystem::exists(ourFilePath)) {
            throw std::runtime_error("Our file '" + ourFilePath + "' does not exist.");
        }
        if (!std::filesystem::exists(theirFilePath)) {
            throw std::runtime_error("Their file '" + theirFilePath + "' does not exist.");
        }

        // TODO: Parse extension and then the compute model based on that, create a Format Factory
        // For now, assume .obj file format
        const auto [baseModel, ourModel, theirModel] = loadObjModels(
            std::array{baseFilePath, ourFilePath, theirFilePath}, {"Base model", "Our model", "Their model"});
        Delta ourDelta = MeshDiff::computeDelta(baseModel, ourModel);
        Delta theirDelta = MeshDiff::computeDelta(baseModel, theirModel);

        Viewer viewer;
        const std::array<const std::vector<std::array<float, 3>>*, 3> verticePtrs = {
            &baseModel.getVertices(), &ourModel.getVertices(), &theirModel.getVertices()};
        const std::array<const std::vector<std::vector<std::size_t>>*, 6> indicePtrs = {
            &ourDelta.common,   &ourDelta.added,   &ourDelta.removed,
            &theirDelta.common, &theirDelta.added, &theirDelta.removed};

        ComparisonSession session(verticePtrs, indicePtrs, "merge");
        if (benchmark) return 0;
        if (!viewer.run(session)) throw std::runtime_error("Merge canceled; no output was saved.");

        if (session.isResolved()) {
            auto [mergedVertices, mergedFaces] = session.getMergedGeometry();

            PolygonMesh model;
            for (const auto& v : mergedVertices) model.addVertex(v);
            for (const auto& f : mergedFaces) model.addFace(f);

            ObjIO::write(outputFilePath.empty() ? ourFilePath : outputFilePath, model);
        } else {
            throw std::runtime_error("Merge aborted with " + std::to_string(session.getTotalConflicts()) +
                                     " conflicts left unresolved.");
        }

        return 0;
    }
} // namespace Morf
