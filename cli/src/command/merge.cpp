// morf - cli/src/command/merge.cpp
#include "application/application.hpp"
#include "application/session.hpp"
#include "command/merge.hpp"
#include "compare/engine.hpp"
#include "formats/obj/handler.hpp"
#include "formats/model.hpp"
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

    int Merge::run(const std::string& baseFilePath, const std::string& ourFilePath, const std::string& theirFilePath) {
        if (!std::filesystem::exists(baseFilePath))   { throw std::runtime_error("Base file '" + baseFilePath + "' does not exist."); }
        if (!std::filesystem::exists(ourFilePath))    { throw std::runtime_error("Our file '" + ourFilePath + "' does not exist."); }
        if (!std::filesystem::exists(theirFilePath))  { throw std::runtime_error("Their file '" + theirFilePath + "' does not exist."); }

        // TODO: Parse extension and then the compute model based on that, create a Format Factory
        // For now, assume .obj file format
        Engine engine;
        GenericModel baseModel  = Handler::convert(Handler::parse(baseFilePath));
        GenericModel ourModel   = Handler::convert(Handler::parse(ourFilePath));
        GenericModel theirModel = Handler::convert(Handler::parse(theirFilePath));
        Delta ourDelta   = engine.computeDelta(baseModel, ourModel);
        Delta theirDelta = engine.computeDelta(baseModel, theirModel);

        Application app;
        const std::array<const std::vector<std::array<float, 3>>*, 3> verticePtrs = {
            &baseModel.getVertices(),
            &ourModel.getVertices(),
            &theirModel.getVertices()
        };
        const std::array<const std::vector<std::vector<std::size_t>>*, 6> indicePtrs = {
            &ourDelta.common,
            &ourDelta.added,
            &ourDelta.removed,
            &theirDelta.common,
            &theirDelta.added,
            &theirDelta.removed
        };

        Session session(verticePtrs, indicePtrs, "merge");
        app.run(session);

        if (session.isResolved()) {
            auto [mergedVertices, mergedFaces] = session.getMergedGeometry();

            GenericModel model;
            for (const auto& v : mergedVertices) model.addVertex(v);
            for (const auto& f : mergedFaces)    model.addFace(f);

            Handler::write(ourFilePath, model);
        } else {
            throw std::runtime_error("Merge aborted with " + std::to_string(session.getTotalConflicts()) + " conflicts left unresolved.");
            return 1;
        }

        return 0;
    }
} // namespace Morf