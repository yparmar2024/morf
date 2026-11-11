#pragma once
#include "morf/geometry/mesh.hpp"
#include "morf/io/temporary_file.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace Morf::Test {
    class Workspace {
            TemporaryFile owner_{std::filesystem::temp_directory_path()};

        public:
            std::filesystem::path path(const std::string& name) const { return owner_.path().parent_path() / name; }
            std::filesystem::path write(const std::string& name, const std::string& contents) const {
                const auto destination = path(name);
                std::ofstream stream(destination, std::ios::binary);
                stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
                stream.close();
                if (!stream) throw std::runtime_error("Could not write test fixture.");
                return destination;
            }
    };

    inline std::string read(const std::filesystem::path& path) {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) throw std::runtime_error("Could not read test output.");
        return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    }

    inline PolygonMesh triangle(float offset = 0) {
        PolygonMesh mesh;
        mesh.addVertex({offset, 0, 0});
        mesh.addVertex({offset + 1, 0, 0});
        mesh.addVertex({offset, 1, 0});
        mesh.addFace({0, 1, 2});
        return mesh;
    }
} // namespace Morf::Test
