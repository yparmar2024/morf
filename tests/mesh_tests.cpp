#include "morf/compare/mesh_diff.hpp"
#include "test_support.hpp"

#include <catch2/catch_test_macros.hpp>
#include <limits>

using namespace Morf;
using namespace Morf::Test;

TEST_CASE("Identical meshes have no added or removed faces", "[diff]") {
    const auto mesh = triangle();
    const auto delta = MeshDiff::computeDelta(mesh, mesh);
    REQUIRE(delta.common == std::vector<std::vector<std::size_t>>{{0, 1, 2}});
    REQUIRE(delta.added.empty());
    REQUIRE(delta.removed.empty());
}

TEST_CASE("Vertex numbering and cyclic face rotation do not change geometry", "[diff]") {
    auto base = triangle();
    PolygonMesh reordered;
    reordered.addVertex({0, 1, 0});
    reordered.addVertex({0, 0, 0});
    reordered.addVertex({1, 0, 0});
    reordered.addFace({2, 0, 1});
    const auto delta = MeshDiff::computeDelta(base, reordered);
    REQUIRE(delta.common == std::vector<std::vector<std::size_t>>{{0, 1, 2}});
    REQUIRE(delta.added.empty());
    REQUIRE(delta.removed.empty());
}

TEST_CASE("Reversing winding is a face change", "[diff]") {
    auto base = triangle();
    PolygonMesh reversed;
    for (const auto& v : base.getVertices()) reversed.addVertex(v);
    reversed.addFace({0, 2, 1});
    const auto delta = MeshDiff::computeDelta(base, reversed);
    REQUIRE(delta.common.empty());
    REQUIRE(delta.added == std::vector<std::vector<std::size_t>>{{0, 2, 1}});
    REQUIRE(delta.removed == std::vector<std::vector<std::size_t>>{{0, 1, 2}});
}

TEST_CASE("Added and removed faces refer to the correct model", "[diff]") {
    auto base = triangle();
    auto target = triangle(10);
    const auto delta = MeshDiff::computeDelta(base, target);
    REQUIRE(delta.common.empty());
    REQUIRE(delta.removed == std::vector<std::vector<std::size_t>>{{0, 1, 2}});
    REQUIRE(delta.added == std::vector<std::vector<std::size_t>>{{0, 1, 2}});
    REQUIRE(target.getVertices()[delta.added[0][0]][0] == 10);
    REQUIRE(base.getVertices()[delta.removed[0][0]][0] == 0);
}

TEST_CASE("Empty meshes support additions and deletions", "[diff]") {
    const PolygonMesh empty;
    const auto mesh = triangle();
    REQUIRE(MeshDiff::computeDelta(empty, mesh).added.size() == 1);
    REQUIRE(MeshDiff::computeDelta(mesh, empty).removed.size() == 1);
    REQUIRE(MeshDiff::computeDelta(empty, empty).common.empty());
}

TEST_CASE("Canonicalization validates coordinates and face references", "[diff]") {
    PolygonMesh invalidIndex;
    invalidIndex.addVertex({0, 0, 0});
    invalidIndex.addFace({0, 1, 2});
    PolygonMesh shortFace;
    shortFace.addVertex({0, 0, 0});
    shortFace.addFace({0, 0});
    PolygonMesh nonfinite;
    nonfinite.addVertex({std::numeric_limits<float>::infinity(), 0, 0});
    for (const auto& mesh : {invalidIndex, shortFace, nonfinite}) {
        bool rejected = false;
        try {
            mesh.getCanonicalForm();
        } catch (const std::runtime_error&) {
            rejected = true;
        }
        REQUIRE(rejected);
    }
}
