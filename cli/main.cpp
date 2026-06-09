#include "../core/include/parser.hpp"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    // Parse args, throw error if two paths to models are not provided
    if (argc != 3) {
        std::cerr << "Usage: polydiff <base_model.obj> <new_model.obj>" << std::endl;
        return 1;
    }

    // Parse model paths
    std::string basePath = argv[1];
    std::string newPath = argv[2];
    
    // Validate model paths
    if (!std::filesystem::exists(basePath)) {
        std::cerr << "Error: File '" << basePath << "' does not exist." << std::endl;
        return 1;
    }
    if (!std::filesystem::exists(newPath)) {
        std::cerr << "Error: File '" << newPath << "' does not exist." << std::endl;
        return 1;
    }

    // Create model via parser
    Polydiff::Model model = Polydiff::Parser::parse(basePath);

    // TODO: Perform diff logic

    return 0;
}