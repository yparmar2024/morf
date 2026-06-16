// morf - core/src/parser.cpp
#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <string>

namespace Morf {
    Model Parser::parse(const std::string& filePath) {
        Model model;
        std::ifstream file(filePath);
        std::string line;
        Object* object = nullptr;
        std::unordered_map<std::string, int> unknownTokens;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type[0] == '#') {
                // Skip comments
                continue;
            } else if (type == "v") {
                float x, y, z;
                ss >> x >> y >> z;
                model.vertices.push_back({x, y, z});
            } else if (type == "vt") {
                float u, v, w = 0.0f;
                ss >> u >> v >> w;
                model.textureVertices.push_back({u, v, w});
            } else if (type == "vn") {
                float x, y, z;
                ss >> x >> y >> z;
                model.normalVertices.push_back({x, y, z});
            } else if (type == "f") {
                // If no object exists, create a 'default' one
                if (object == nullptr) {
                    model.objects.push_back({"default", {}});
                    object = &model.objects.back();
                }
                Face face;
                std::string token;

                while (ss >> token) {
                    VertexData vData;
                    std::stringstream ts(token);
                    std::string part;
                    int slot = 0;

                    while (std::getline(ts, part, '/')) {
                        if (!part.empty()) {
                            int val = std::stoi(part) - 1;
                            if (slot == 0) {
                                vData.vIdx = val;
                            } else if (slot == 1) {
                                vData.vtIdx = val;
                            } else if (slot == 2) {
                                vData.vnIdx = val;
                            }
                        }
                        slot++;
                    }
                    face.vertexData.push_back(vData);
                }
                object->faces.push_back(face);
            } else if (type == "o") {
                std::string name;
                ss >> name;
                model.objects.push_back({name, {}});
                object = &model.objects.back();
            } else {
                // Accumulate unknown first tokens
                unknownTokens[type]++;
            }
        }

        if (!unknownTokens.empty()) {
            std::cerr << "\nWarning: unrecognized OBJ commands/tokens found for " << filePath << ":\n";
            for (const auto& [token, count] : unknownTokens) { std::cerr << "  " << token << " (" << count << " occurences\n"; }
            std::cerr << "\n" << std::endl;
        }
        return model;
    };
} // namespace Morf