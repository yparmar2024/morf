#include <iostream>

int main(int argc, char* argv[]) {
    // Parse args, throw error if two paths to models are not provided
    if (argc != 3) {
        std::cerr << "Usage: polydiff <base_model.obj> <new_model.obj>" << std::endl;
        return 1;
    }

    // Parse model paths
    std::string basePath = argv[1];
    std::string newPath = argv[2];

    // TODO: Perform parser logic
    // TODO: Perform diff logic

    return 0;
}