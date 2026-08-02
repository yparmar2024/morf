// morf - core/src/formats/obj/handler.cpp
#include "formats/obj/handler.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace Morf {
    Model Handler::parse(const std::string& filePath) {
        Model model;
        std::ifstream file(filePath);
        std::string line;
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
                model.addVertex({x, y, z});
            } else if (keyword == "vt") {
                float u, v, w = 0.0f;
                ss >> u >> v >> w;
                model.addTextureVertex({u, v, w});
            } else if (keyword == "vn") {
                float x, y, z;
                ss >> x >> y >> z;
                model.addNormalVertex({x, y, z});
            } else if (keyword == "f") {
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
                                if (slot == 0)      idx = (int)model.getVertices().size() + raw;
                                else if (slot == 1) idx = (int)model.getTextureVertices().size() + raw;
                                else if (slot == 2) idx = (int)model.getNormalVertices().size() + raw;
                            }

                            int maxSize;
                            if (slot == 0)      maxSize = (int)model.getVertices().size();
                            else if (slot == 1) maxSize = (int)model.getTextureVertices().size();
                            else if (slot == 2) maxSize = (int)model.getNormalVertices().size();

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
                    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
                    const auto& verts = model.getVertices();
                    const auto& v0 = verts[face.vertexData[0].vIdx];

                    for (std::size_t k = 1; k < face.vertexData.size() - 1; ++k) {
                        const auto& v1 = verts[face.vertexData[k].vIdx];
                        const auto& v2 = verts[face.vertexData[k + 1].vIdx];

                        float e1x = v1.x - v0.x, e1y = v1.y - v0.y, e1z = v1.z - v0.z;
                        float e2x = v2.x - v0.x, e2y = v2.y - v0.y, e2z = v2.z - v0.z;

                        nx += (e1y * e2z - e1z * e2y);
                        ny += (e1z * e2x - e1x * e2z);
                        nz += (e1x * e2y - e1y * e2x);
                    }

                    if ((nx * nx + ny * ny + nz * nz) == 0) {
                        invalidArgs["degenerate (zero-area/collinear) faces"]++;
                    } else {
                        model.addFace(face);
                    }
                }
            } else if (keyword == "o") {
                std::string name;
                ss >> name;
                model.addObject({name, {}});
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
    }

    GenericModel Handler::convert(const Model& model) {
        GenericModel genericModel;

        for (const auto& v : model.getVertices()) genericModel.addVertex({v.x, v.y, v.z});
        for (const auto& o : model.getObjects()) {
            for (const auto& f : o.faces) {
                std::vector<std::size_t> fData;
                for (const auto& vData : f.vertexData) {
                    fData.push_back(vData.vIdx);
                }
                genericModel.addFace(fData);
            }
        }
        return genericModel;
    }

    void Handler::write(const std::string& filePath, const GenericModel& model) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "ERROR: Failed to open '" << filePath << "' for writing.\n";
            return;
        }

        for (const auto& v : model.getVertices()) {
            file << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
        }
        for (const auto& f : model.getFaces()) {
            file << "f";
            for (std::size_t idx : f) {
                file << " " << (idx + 1);
            }
            file << "\n";
        }
        file.close();
    }
} // namespace Morf