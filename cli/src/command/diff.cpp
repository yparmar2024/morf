// morf - cli/src/command/diff.cpp
#include "command/diff.hpp"
#include "command/load_models.hpp"
#include "morf/compare/mesh_diff.hpp"
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
    std::string Diff::getExtension(const std::string& filePath) {
        std::string ext = std::filesystem::path(filePath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
        return ext;
    }

    int Diff::run(const std::string& baseFilePath, const std::string& targetFilePath, bool benchmark) {
        if (!std::filesystem::exists(baseFilePath)) {
            throw std::runtime_error("Base file '" + baseFilePath + "' does not exist.");
        }
        if (!std::filesystem::exists(targetFilePath)) {
            throw std::runtime_error("Target file '" + targetFilePath + "' does not exist.");
        }

        // TODO: Parse extension and then the compute model based on that, create a Format Factory
        // For now, assume .obj file format
        const auto [baseModel, targetModel] =
            loadObjModels(std::array{baseFilePath, targetFilePath}, {"Base model", "Target model"});
        Delta delta = MeshDiff::computeDelta(baseModel, targetModel);

        Viewer viewer;
        const std::array<const std::vector<std::array<float, 3>>*, 3> verticePtrs = {
            &baseModel.getVertices(), &baseModel.getVertices(), &targetModel.getVertices()};
        const std::array<const std::vector<std::vector<std::size_t>>*, 6> indicePtrs = {
            nullptr, nullptr, nullptr, &delta.common, &delta.added, &delta.removed};

        ComparisonSession session(verticePtrs, indicePtrs, "diff");
        if (benchmark) return 0;
        viewer.run(session);

        return 0;
    }
} // namespace Morf
