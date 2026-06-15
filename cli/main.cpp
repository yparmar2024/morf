#include "parser.hpp"
#include "diff.hpp"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    // Parse args, throw error if two paths to models are not provided
    if (argc != 3) {
        std::cerr << "Usage: morf <base_model.obj> <target_model.obj>" << std::endl;
        return 1;
    }

    // Parse model paths
    std::string basePath = argv[1];
    std::string targetPath = argv[2];
    
    // Validate model paths
    if (!std::filesystem::exists(basePath)) {
        std::cerr << "Error: File '" << basePath << "' does not exist." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(targetPath)) {
        std::cerr << "Error: File '" << targetPath << "' does not exist." << std::endl;
        return 1;
    }

    // Create models via parser
    Morf::Model baseModel = Morf::Parser::parse(basePath);
    Morf::Model targetModel = Morf::Parser::parse(targetPath);

    // Call compare on the models
    Morf::Diff diff = Morf::DiffEngine::compare(baseModel, targetModel);

    // TODO: Visualize difference via Raylib integration

    return 0;
}