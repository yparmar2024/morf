// morf - cli/main.cpp
#include "application.hpp"
#include "parser.hpp"
#include "diff.hpp"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: morf <base_model.obj> <target_model.obj>" << std::endl;
        return 1;
    }

    std::string basePath = argv[1];
    std::string targetPath = argv[2];
    
    if (!std::filesystem::exists(basePath)) {
        std::cerr << "Error: File '" << basePath << "' does not exist." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(targetPath)) {
        std::cerr << "Error: File '" << targetPath << "' does not exist." << std::endl;
        return 1;
    }

    Morf::Model baseModel = Morf::Parser::parse(basePath);
    Morf::Model targetModel = Morf::Parser::parse(targetPath);
    Morf::Diff diff = Morf::DiffEngine::compare(baseModel, targetModel);

    Morf::Application app(1600, 900);
    app.isMergeMode = true;
    app.run(baseModel, targetModel, diff);

    return 0;
}