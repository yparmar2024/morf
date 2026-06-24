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
        std::unordered_map<std::string, int> unknownKeywords;
        std::unordered_map<std::string, int> invalidArgs;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword.empty() || keyword[0] == '#') {
                continue;
            } else if (keyword == "v") {
                float x, y, z;
                ss >> x >> y >> z;
                model.vertices.push_back({x, y, z});
            } else if (keyword == "vt") {
                float u, v, w = 0.0f;
                ss >> u >> v >> w;
                model.textureVertices.push_back({u, v, w});
            } else if (keyword == "vn") {
                float x, y, z;
                ss >> x >> y >> z;
                model.normalVertices.push_back({x, y, z});
            } else if (keyword == "f") {
                // If no object has been created yet, create a 'default' one
                if (objectIdx == SIZE_MAX) {
                    model.objects.push_back({"default", {}});
                    objectIdx = model.objects.size() - 1;
                }

                Face face;
                std::string token;
                bool validFace = true;
                while (ss >> token) {
                    VertexData vData;
                    std::stringstream ts(token);
                    std::string part;
                    int slot = 0;

                    while (std::getline(ts, part, '/')) {
                        if (!part.empty()) {
                            int raw = std::stoi(part);
                            if (raw == 0) { validFace = false; break; }

                            // Relative index normalization
                            int idx;
                            if (raw > 0) {
                                idx = raw - 1;
                            } else {
                                if (slot == 0)      idx = (int)model.vertices.size() + raw;
                                else if (slot == 1) idx = (int)model.textureVertices.size() + raw;
                                else if (slot == 2) idx = (int)model.normalVertices.size() + raw;
                            }

                            int maxSize;
                            if (slot == 0)      maxSize = (int)model.vertices.size();
                            else if (slot == 1) maxSize = (int)model.textureVertices.size();
                            else if (slot == 2) maxSize = (int)model.normalVertices.size();

                            if (idx < 0 || idx >= maxSize) { validFace = false; break; }

                            if (slot == 0)      vData.vIdx = idx;
                            else if (slot == 1) vData.vtIdx = idx;
                            else if (slot == 2) vData.vnIdx = idx;
                        }
                        slot++;
                    }
                    if (validFace) face.vertexData.push_back(vData);    
                    else           break;
                }
                if (!validFace) {
                    invalidArgs["invalid vertex indices"]++;
                } else if (face.vertexData.size() < 3) {
                    invalidArgs["faces with less than 3 vertices"]++;
                } else {
                    face.materialName = currMaterialName;
                    model.objects[objectIdx].faces.push_back(face);
                }
            } else if (keyword == "o") {
                std::string name;
                ss >> name;
                model.objects.push_back({name, {}});
                objectIdx = model.objects.size() - 1;
            } else if (keyword == "mtllib") {
                std::string mtlFilename;
                ss >> mtlFilename;
                std::string dirPath = filePath.substr(0, filePath.find_last_of("/\\") + 1);
                model.materials = parseMtl(dirPath + mtlFilename);
            } else if (keyword == "usemtl") {
                ss >> currMaterialName;
            } else {
                unknownKeywords[keyword]++;
            }
        }

        if (!unknownKeywords.empty()) {
            std::cerr << "WARNING: Unrecognized OBJ keywords found for " << filePath << ":\n";
            for (const auto& [keyword, count] : unknownKeywords) { std::cerr << "WARNING:     > " << keyword << " (" << count << " occurences)\n"; }
        }
        if (!invalidArgs.empty()) {
            std::cerr << "WARNING: Invalid OBJ args found for " << filePath << ":\n";
            for (const auto& [arg, count] : invalidArgs) { std::cerr << "WARNING:     > " << arg << " (" << count << " occurences)\n"; }
        }
        return model;
    };

    std::unordered_map<std::string, Material> Parser::parseMtl(const std::string& filePath) {
        std::unordered_map<std::string, Material> materials;
        std::ifstream file(filePath);

        if (!file.is_open()) {
            std::cerr << "WARNING: Could not open MTL file '" << filePath << "'";
            return materials;
        }
        
        std::string line;
        Material material;
        std::string name;
        bool hasMaterial = false;
        std::unordered_map<std::string, int> unknownKeywords;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword.empty() || keyword[0] == '#') {
                continue;
            } else if (keyword == "newmtl") {
                if (hasMaterial) materials[name] = material;
                ss >> name;
                material = Material();
                hasMaterial = true;
            } else if (keyword == "Kd" && hasMaterial) {
                float r, g, b;
                ss >> r >> g >> b;
                material.diffuse = { r, g, b };
            } else if (keyword == "Ka" && hasMaterial) {
                float r, g, b;
                ss >> r >> g >> b;
                material.ambient = { r, g, b };
            } else if (keyword == "Ks" && hasMaterial) {
                float r, g, b;
                ss >> r >> g >> b;
                material.specular = { r, g, b };
            } else if (keyword == "Ns" && hasMaterial) {
                ss >> material.exponent;
            } else if (keyword == "map_Kd" && hasMaterial) {
                ss >> material.textureMap;
            } else {
                unknownKeywords[keyword]++;
            }
        }

        if (hasMaterial) materials[name] = material;
        if (!unknownKeywords.empty()) {
            std::cerr << "WARNING: unrecognized MTL keywords found for " << filePath << ":\n";
            for (const auto& [keyword, count] : unknownKeywords) { std::cerr << "WARNING:     > " << keyword << " (" << count << " occurences)\n"; }
        }
        return materials;
    };
} // namespace Morf