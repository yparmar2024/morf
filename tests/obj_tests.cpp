#include "morf/formats/obj/obj_io.hpp"
#include "test_support.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <sstream>
#include <string>

using namespace Morf;
using namespace Morf::Test;

TEST_CASE("OBJ parses positions, UVs, normals, and object names", "[obj]") {
    const auto model = ObjIO::parse((std::filesystem::path(MORF_FIXTURES_DIR) / "triangle.obj").string());
    REQUIRE(model.getVertices().size() == 3);
    REQUIRE(model.getVertices()[1].x == 1);
    REQUIRE(model.getTextureVertices().size() == 3);
    REQUIRE(model.getTextureVertices()[2].v == 1);
    REQUIRE(model.getNormalVertices().size() == 1);
    REQUIRE(model.getNormalVertices()[0].z == 1);
    REQUIRE(model.getObjects().size() == 1);
    REQUIRE(model.getObjects()[0].name == "triangle");
    const auto& face = model.getObjects()[0].faces.at(0).vertexData;
    REQUIRE(face.size() == 3);
    REQUIRE(face[2].vIdx == 2);
    REQUIRE(face[2].vtIdx == 2);
    REQUIRE(face[2].vnIdx == 0);
}

TEST_CASE("OBJ handles line endings, comments, and a final unterminated record", "[obj]") {
    Workspace work;
    for (const auto& ending : {"\n", "\r\n"}) {
        const std::string e = ending;
        const auto file = work.write("input.obj", "# heading" + e + e + "\tv 0 0 0 # first" + e + "v 1 0 0" + e +
                                                      "v 0 1 0" + e + "f -3 -2 -1 # last");
        std::ostringstream diagnostics;
        const auto model = ObjIO::parse(file.string(), diagnostics);
        REQUIRE(diagnostics.str().empty());
        REQUIRE(model.getVertices().size() == 3);
        REQUIRE(model.getObjects()[0].faces[0].vertexData[2].vIdx == 2);
    }
}

TEST_CASE("OBJ supports each face reference form", "[obj]") {
    Workspace work;
    const std::string prefix = "v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0\nvt 0 1\nvt 0 1 2\nvn 0 0 1\n";
    for (const auto& record : {"f +1 +2 +3", "f 1/1 2/2 3/3", "f 1//1 2//1 3//1", "f 1/1/1 2/2/1 3/3/1"}) {
        std::ostringstream diagnostics;
        const auto model = ObjIO::parse(work.write("face.obj", prefix + record).string(), diagnostics);
        REQUIRE(diagnostics.str().empty());
        REQUIRE(model.getObjects().size() == 1);
        REQUIRE(model.getObjects()[0].faces.size() == 1);
    }
}

TEST_CASE("OBJ rejects malformed coordinate records with location", "[obj]") {
    Workspace work;
    for (const auto& record : {"v 1 2", "vn x 0 0", "vt .5 x", "v 1e999 0 0"}) {
        const auto file = work.write("bad.obj", std::string("# heading\n") + record);
        try {
            ObjIO::parse(file.string());
            FAIL("Malformed record was accepted");
        } catch (const std::runtime_error& error) {
            REQUIRE(std::string(error.what()).find(file.string()) != std::string::npos);
            REQUIRE(std::string(error.what()).find("line 2") != std::string::npos);
        }
    }
}

TEST_CASE("OBJ warns and skips invalid or degenerate faces", "[obj]") {
    Workspace work;
    const std::string prefix = "v 0 0 0\nv 1 0 0\nv 0 1 0\n";
    for (const auto& record : {"f 0 2 3", "f 4 2 3", "f -4 2 3", "f 1x 2 3", "f 2147483648 2 3", "f 1/ 2 3",
                               "f 1/1/1/1 2 3", "f 1 2", "f 1 1 1"}) {
        CAPTURE(record);
        std::ostringstream diagnostics;
        const auto model = ObjIO::parse(work.write("bad-face.obj", prefix + record).string(), diagnostics);
        REQUIRE(model.getObjects().empty());
        REQUIRE(diagnostics.str().find("Invalid OBJ args") != std::string::npos);
    }
}

TEST_CASE("OBJ accepts empty files and creates a default object for faces", "[obj]") {
    Workspace work;
    REQUIRE(ObjIO::parse(work.write("empty.obj", "").string()).getVertices().empty());
    const auto model = ObjIO::parse(work.write("default.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3").string());
    REQUIRE(model.getObjects()[0].name == "default");
}

TEST_CASE("OBJ conversion retains polygon geometry across objects", "[obj]") {
    Workspace work;
    const auto model = ObjIO::parse(
        work.write("objects.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 1 1 0\no first\nf 1 2 3\no second\nf 2 4 3").string());
    const auto mesh = ObjIO::convert(model);
    REQUIRE(mesh.getVertices().size() == 4);
    REQUIRE(mesh.getFaces() == std::vector<std::vector<std::size_t>>{{0, 1, 2}, {1, 3, 2}});
}

TEST_CASE("OBJ extracts geometry from a multi-material document", "[obj]") {
    std::ostringstream diagnostics;
    const auto model = ObjIO::parse((std::filesystem::path(MORF_FIXTURES_DIR) / "materials.obj").string(), diagnostics);
    const auto mesh = ObjIO::convert(model);
    REQUIRE(mesh.getVertices().size() == 6);
    REQUIRE(mesh.getFaces() == std::vector<std::vector<std::size_t>>{{0, 1, 2}, {3, 4, 5}});
    // Material assignments will be checked when keyword preservation is implemented.
}
