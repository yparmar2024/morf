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
                // Fetch x, y, and z and push vertex
                float x, y, z;
                ss >> x >> y >> z;
                model.vertices.push_back({x, y, z});
            } else if (type == "f") {
                // If no object exists yet, create a default one
                if (object == nullptr) {
                    model.objects.push_back({"default", {}});
                    object = &model.objects.back();
                }

                // Declare index and face
                int idx;
                Face face;

                // Fetch each index and push to faces
                while (ss >> idx) {
                    VertexData vData;
                    vData.vIdx = idx - 1;
                    face.vertices.push_back(vData);
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