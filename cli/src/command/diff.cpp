// morf - cli/src/command/diff.cpp
#include "application/application.hpp"
#include "application/session.hpp"
#include "command/diff.hpp"
#include "command/load_models.hpp"
#include "compare/engine.hpp"
#include "formats/model.hpp"

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
        Engine engine;
        const auto [baseModel, targetModel] =
            loadObjModels(std::array{baseFilePath, targetFilePath}, {"Base model", "Target model"});
        Delta delta = engine.computeDelta(baseModel, targetModel);

        Application app;
        const std::array<const std::vector<std::array<float, 3>>*, 3> verticePtrs = {
            &baseModel.getVertices(), &baseModel.getVertices(), &targetModel.getVertices()};
        const std::array<const std::vector<std::vector<std::size_t>>*, 6> indicePtrs = {
            nullptr, nullptr, nullptr, &delta.common, &delta.added, &delta.removed};

        Session session(verticePtrs, indicePtrs, "diff");
        if (benchmark) return 0;
        app.run(session);

        return 0;
    }
} // namespace Morf
