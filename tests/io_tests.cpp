#include "morf/formats/obj/obj_io.hpp"
#include "morf/io/mapped_file.hpp"
#include "morf/performance/timing.hpp"
#include "test_support.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <filesystem>
#include <limits>
#include <sstream>
#include <string_view>
#include <sys/stat.h>

using namespace Morf;
using namespace Morf::Test;

TEST_CASE("Mapped lines retain views and respect EOF", "[io]") {
    Workspace work;
    MappedFile file(work.write("lines.obj", "first\n\nthird").string());
    std::string_view first, next;
    REQUIRE(file.readLine(first));
    REQUIRE(first == "first");
    REQUIRE(file.readLine(next));
    REQUIRE(next.empty());
    REQUIRE(file.readLine(next));
    REQUIRE(next == "third");
    REQUIRE(first == "first");
    REQUIRE_FALSE(file.readLine(next));
    REQUIRE(next.empty());
}

TEST_CASE("Mapping handles empty and page-spanning input", "[io]") {
    Workspace work;
    const auto text = std::string(65537, 'x');
    MappedFile file(work.write("large.obj", text).string());
    std::string_view line;
    REQUIRE(file.readLine(line));
    REQUIRE(line == text);
    REQUIRE_FALSE(file.readLine(line));
    MappedFile empty(work.write("empty.obj", "").string());
    REQUIRE_FALSE(empty.readLine(line));
}

TEST_CASE("Mapping rejects missing files, directories, and FIFOs", "[io]") {
    Workspace work;
    REQUIRE_THROWS(MappedFile(work.path("missing.obj").string()));
    REQUIRE_THROWS(MappedFile(work.path("").string()));
    const auto pipe = work.path("pipe");
    REQUIRE(::mkfifo(pipe.c_str(), 0600) == 0);
    REQUIRE_THROWS(MappedFile(pipe.string()));
}

TEST_CASE("Temporary files clean up only their owned directory", "[io]") {
    Workspace work;
    const auto unrelated = work.write("keep.obj", "keep");
    std::filesystem::path owned;
    {
        TemporaryFile temporary(work.path(""));
        owned = temporary.path().parent_path();
        REQUIRE(std::filesystem::exists(owned));
        struct stat status{};
        REQUIRE(::stat(owned.c_str(), &status) == 0);
        REQUIRE((status.st_mode & 0777) == 0700);
        std::ofstream(temporary.path()) << "temporary";
    }
    REQUIRE_FALSE(std::filesystem::exists(owned));
    REQUIRE(read(unrelated) == "keep");
}

TEST_CASE("Writing geometry preserves float values and destination permissions", "[io]") {
    Workspace work;
    PolygonMesh mesh;
    mesh.addVertex({std::nextafter(1.0f, 2.0f), 0, 0});
    mesh.addVertex({0, 1, 0});
    mesh.addVertex({0, 0, 1});
    mesh.addFace({0, 1, 2});
    const auto output = work.write("saved.obj", "previous");
    const auto mode = std::filesystem::perms::owner_read | std::filesystem::perms::owner_write;
    std::filesystem::permissions(output, mode);
    ObjIO::write(output.string(), mesh);
    const auto restored = ObjIO::convert(ObjIO::parse(output.string()));
    REQUIRE(restored.getVertices() == mesh.getVertices());
    REQUIRE(restored.getFaces() == mesh.getFaces());
    REQUIRE(std::filesystem::status(output).permissions() == mode);
}

TEST_CASE("A failed write preserves the destination", "[io]") {
    Workspace work;
    const auto destination = work.path("directory");
    std::filesystem::create_directory(destination);
    const auto marker = destination / "keep";
    std::ofstream(marker) << "keep";
    REQUIRE_THROWS(ObjIO::write(destination.string(), triangle()));
    REQUIRE(read(marker) == "keep");
    REQUIRE(std::distance(std::filesystem::directory_iterator(work.path("")), std::filesystem::directory_iterator{}) ==
            1);
}

TEST_CASE("Timing reports are opt-in and emitted once", "[timing]") {
    std::ostringstream output;
    {
        PhaseTimings timings(false);
        ScopedPhase phase("disabled");
        timings.print(output);
        REQUIRE(output.str().empty());
    }
    {
        PhaseTimings timings(true, true);
        {
            ScopedPhase phase("known phase");
        }
        timings.print(output);
        REQUIRE(output.str().find("known phase") != std::string::npos);
        REQUIRE(output.str().find("Status: complete") != std::string::npos);
        const auto once = output.str();
        timings.print(output);
        REQUIRE(output.str() == once);
    }
}

TEST_CASE("Timing reports record an unwound phase", "[timing]") {
    PhaseTimings timings(true);
    try {
        ScopedPhase phase("failing phase");
        throw std::runtime_error("failure");
    } catch (const std::runtime_error&) {
    }
    std::ostringstream output;
    timings.print(output);
    REQUIRE(output.str().find("failing phase [failed]") != std::string::npos);
    REQUIRE(output.str().find("Status: incomplete/failed") != std::string::npos);
}
