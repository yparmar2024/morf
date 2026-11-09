// morf - core/src/formats/obj/handler.cpp
#include "formats/obj/handler.hpp"
#include "io/mapped_file.hpp"
#include "io/temporary.hpp"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace Morf {
    Model Handler::parse(const std::string& filePath) {
        Model model;
        MappedFile file(filePath);
        std::string line;
        std::size_t lineNumber = 0;
        std::unordered_map<std::string, int> unknownKeywords;
        std::unordered_map<std::string, int> invalidArgs;

        while (file.readLine(line)) {
            ++lineNumber;
            // Inline comments must not be parsed as coordinates or face references.
            line.erase(line.find('#') == std::string::npos ? line.size() : line.find('#'));
            std::stringstream ss(line);
            const auto invalidRecord = [&]() {
                return std::runtime_error("Invalid OBJ record in '" + filePath + "' at line " +
                                          std::to_string(lineNumber) + ".");
            };
            std::string keyword;
            ss >> keyword;

            if (keyword.empty() || keyword[0] == '#') {
                continue;
            } else if (keyword == "v") {
                float x, y, z;
                if (!(ss >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
                    throw invalidRecord();
                model.addVertex({x, y, z});
            } else if (keyword == "vt") {
                float u = 0.0f, v = 0.0f, w = 0.0f;
                if (!(ss >> u)) throw invalidRecord();
                ss >> std::ws;
                if (!ss.eof()) {
                    if (!(ss >> v)) throw invalidRecord();
                    ss >> std::ws;
                    if (!ss.eof() && !(ss >> w)) throw invalidRecord();
                }
                if (!std::isfinite(u) || !std::isfinite(v) || !std::isfinite(w)) throw invalidRecord();
                model.addTextureVertex({u, v, w});
            } else if (keyword == "vn") {
                float x, y, z;
                if (!(ss >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
                    throw invalidRecord();
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
                        if (slot >= 3 || (slot == 0 && part.empty())) {
                            validFace = false;
                            break;
                        }
                        if (!part.empty()) {
                            int raw;
                            try {
                                std::size_t consumed = 0;
                                raw = std::stoi(part, &consumed);
                                if (consumed != part.size() || raw == 0) {
                                    validFace = false;
                                    break;
                                }
                            } catch (const std::invalid_argument&) {
                                validFace = false;
                                break;
                            } catch (const std::out_of_range&) {
                                validFace = false;
                                break;
                            }

                            const std::size_t count = slot == 0   ? model.getVertices().size()
                                                      : slot == 1 ? model.getTextureVertices().size()
                                                                  : model.getNormalVertices().size();
                            // VertexData stores int indices; reject unrepresentable input.
                            if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
                                throw invalidRecord();
                            const auto idx =
                                raw > 0 ? static_cast<long long>(raw) - 1 : static_cast<long long>(count) + raw;
                            if (idx < 0 || idx >= static_cast<long long>(count)) {
                                validFace = false;
                                break;
                            }
                            if (slot == 0)
                                vData.vIdx = static_cast<int>(idx);
                            else if (slot == 1)
                                vData.vtIdx = static_cast<int>(idx);
                            else
                                vData.vnIdx = static_cast<int>(idx);
                        }
                        ++slot;
                    }
                    if (vData.vIdx < 0 || token.back() == '/') validFace = false;
                    if (validFace)
                        face.vertexData.push_back(vData);
                    else
                        break;
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
            for (const auto& [keyword, count] : unknownKeywords) {
                std::cerr << "WARNING:     > " << keyword << " (" << count << " occurences)\n";
            }
        }
        if (!invalidArgs.empty()) {
            std::cerr << "WARNING: Invalid OBJ args found for " << filePath << ":\n";
            for (const auto& [arg, count] : invalidArgs) {
                std::cerr << "WARNING:     > " << arg << " (" << count << " occurences)\n";
            }
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
        const std::filesystem::path destination =
            std::filesystem::weakly_canonical(std::filesystem::absolute(filePath));
        TemporaryFile temporary(destination.parent_path());
        std::ofstream file(temporary.path());
        if (!file.is_open()) throw std::runtime_error("Failed to open temporary output for '" + filePath + "'.");
        file << std::setprecision(std::numeric_limits<float>::max_digits10);

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
        if (!file) throw std::runtime_error("Failed while writing '" + filePath + "'.");
        if (std::filesystem::exists(destination))
            std::filesystem::permissions(temporary.path(), std::filesystem::status(destination).permissions());
        std::filesystem::rename(temporary.path(), destination);
    }
} // namespace Morf
