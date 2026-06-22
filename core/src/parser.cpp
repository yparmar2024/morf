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
        std::size_t objectIdx = SIZE_MAX;
        std::string currMaterialName;
        std::unordered_map<std::string, int> unknownTokens;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type.empty() || type[0] == '#') {
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
                // If no object has been created yet, create a 'default' one
                if (objectIdx == SIZE_MAX) {
                    model.objects.push_back({"default", {}});
                    objectIdx = model.objects.size() - 1;
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
                face.mtlName = currMaterialName;
                model.objects[objectIdx].faces.push_back(face);
            } else if (type == "o") {
                std::string name;
                ss >> name;
                model.objects.push_back({name, {}});
                objectIdx = model.objects.size() - 1;
            } else if (type == "mtllib") {
                std::string mtlFilename;
                ss >> mtlFilename;
                std::string dirPath = filePath.substr(0, filePath.find_last_of("/\\") + 1);
                model.materials = parseMtl(dirPath + mtlFilename);
            } else if (type == "usemtl") {
                ss >> currMaterialName;
            } else {
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

    std::unordered_map<std::string, Material> Parser::parseMtl(const std::string& filePath) {
        std::unordered_map<std::string, Material> materials;
        std::ifstream file(filePath);

        if (!file.is_open()) {
            std::cerr << "Warning: Could not open MTL file '" << filePath << "'." << std::endl;
            return materials;
        }
        
        std::string line;
        Material material;
        std::string name;
        bool hasMaterial = false;
        std::unordered_map<std::string, int> unknownTokens;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type.empty() || type[0] == '#') {
                continue;
            } else if (type == "newmtl") {
                if (hasMaterial) materials[name] = material;
                ss >> name;
                material = Material();
                hasMaterial = true;
            } else if (type == "Kd" && hasMaterial) {
                float r, g, b;
                ss >> r >> g >> b;
                material.diffuse = { r, g, b };
            } else if (type == "Ka" && hasMaterial) {
                float r, g, b;
                ss >> r >> g >> b;
                material.ambient = { r, g, b };
            } else if (type == "Ks" && hasMaterial) {
                float r, g, b;
                ss >> r >> g >> b;
                material.specular = { r, g, b };
            } else if (type == "Ns" && hasMaterial) {
                ss >> material.exponent;
            } else if (type == "map_Kd" && hasMaterial) {
                ss >> material.textureMap;
            } else {
                unknownTokens[type]++;
            }
        }

        if (hasMaterial) materials[name] = material;
        if (!unknownTokens.empty()) {
            std::cerr << "\nWarning: unrecognized MTL commands/tokens found for " << filePath << ":\n";
            for (const auto& [token, count] : unknownTokens) { std::cerr << "  " << token << " (" << count << " occurences\n"; }
            std::cerr << "\n" << std::endl;
        }
        return materials;
    };
} // namespace Morf