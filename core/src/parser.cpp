#include "../include/parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Polydiff {
    /* Method to parse an .obj file into a Model object */
    Model Parser::parse(const std::string& filePath) {
        // Declare model, a file stream, current line and current object
        Model model;
        std::ifstream file(filePath);
        std::string line;
        Object* object = nullptr;

        // Iterate over each line in the file
        while (std::getline(file, line)) {
            // Fetch the string stream of the line and declare a type
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type == "v") {
                // Fetch x, y, z and push vertex
                float x, y, z;
                ss >> x >> y >> z;
                model.vertices.push_back({x, y, z});
            } else if (type == "vt") {
                // Fetch u, v, w and push vertex
                float u, v, w = 0.0f;
                ss >> u >> v >> w;
                model.textureVertices.push_back({u, v, w});
            } else if (type == "vn") {
                // Fetch x, y, z and push vertex
                float x, y, z;
                ss >> x >> y >> z;
                model.normalVertices.push_back({x, y, z});
            } else if (type == "f") {
                // If no object exists yet, create a default one
                if (object == nullptr) {
                    model.objects.push_back({"default", {}});
                    object = &model.objects.back();
                }

                // Declare face and token
                Face face;
                std::string token;

                // Fetch each token and update attributes respectively
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
                // Fetch the object name
                std::string name;
                ss >> name;

                // Push to objects and point to active object
                model.objects.push_back({name, {}});
                object = &model.objects.back();
            }
        }

        // Return the parsed model
        return model;
    };
}