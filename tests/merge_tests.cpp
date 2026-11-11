#include "morf/compare/mesh_diff.hpp"
#include "morf/merge/comparison_session.hpp"
#include "test_support.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <utility>

using namespace Morf;
using namespace Morf::Test;

namespace {
    struct MergeCase {
            PolygonMesh base, ours, theirs;
            Delta left, right;
            ComparisonSession session;

            MergeCase(PolygonMesh b, PolygonMesh o, PolygonMesh t)
                : base(std::move(b)), ours(std::move(o)), theirs(std::move(t)),
                  left(MeshDiff::computeDelta(base, ours)), right(MeshDiff::computeDelta(base, theirs)),
                  session({&base.getVertices(), &ours.getVertices(), &theirs.getVertices()},
                          {&left.common, &left.added, &left.removed, &right.common, &right.added, &right.removed},
                          "merge") {}
            MergeCase(const MergeCase&) = delete;
            MergeCase& operator=(const MergeCase&) = delete;
    };
} // namespace

TEST_CASE("An unchanged merge preserves a simple mesh", "[merge]") {
    MergeCase merge(triangle(), triangle(), triangle());
    REQUIRE(merge.session.isResolved());
    const auto [vertices, faces] = merge.session.getMergedGeometry();
    REQUIRE(vertices == merge.base.getVertices());
    REQUIRE(faces == merge.base.getFaces());
}

TEST_CASE("An unchanged side preserves the changed side", "[merge]") {
    bool changeOurs = GENERATE(true, false);
    MergeCase merge(triangle(), triangle(changeOurs ? 10 : 0), triangle(changeOurs ? 0 : 10));
    REQUIRE(merge.session.isResolved());
    const auto [vertices, faces] = merge.session.getMergedGeometry();
    REQUIRE(vertices == triangle(10).getVertices());
    REQUIRE(faces == triangle(10).getFaces());
}

TEST_CASE("Identical edits are included once", "[merge]") {
    MergeCase merge(triangle(), triangle(10), triangle(10));
    REQUIRE(merge.session.isResolved());
    const auto [vertices, faces] = merge.session.getMergedGeometry();
    REQUIRE(vertices == triangle(10).getVertices());
    REQUIRE(faces.size() == 1);
}

TEST_CASE("Independent additions from each side survive", "[merge]") {
    MergeCase merge({}, triangle(), triangle(10));
    REQUIRE(merge.session.isResolved());
    const auto [vertices, faces] = merge.session.getMergedGeometry();
    REQUIRE(vertices.size() == 6);
    REQUIRE(faces == std::vector<std::vector<std::size_t>>{{0, 1, 2}, {3, 4, 5}});
    REQUIRE(vertices[0][0] == 0);
    REQUIRE(vertices[3][0] == 10);
}

TEST_CASE("Overlapping alternatives can be resolved with either side", "[merge]") {
    bool chooseOurs = GENERATE(true, false);
    MergeCase merge({}, triangle(), triangle(.2f));
    REQUIRE_FALSE(merge.session.isResolved());
    REQUIRE(merge.session.getTotalConflicts() == 2);
    while (!merge.session.isResolved()) {
        if (chooseOurs)
            merge.session.acceptOurs();
        else
            merge.session.acceptTheirs();
    }
    const auto [vertices, faces] = merge.session.getMergedGeometry();
    REQUIRE(vertices == triangle(chooseOurs ? 0 : .2f).getVertices());
    REQUIRE(faces.size() == 1);
}

TEST_CASE("Conflict navigation wraps and automatic merging keeps selection valid", "[merge]") {
    auto ours = triangle(), theirs = triangle(.2f);
    const auto secondOurs = triangle(10);
    const auto secondTheirs = triangle(10.2f);
    for (const auto& v : secondOurs.getVertices()) ours.addVertex(v);
    for (const auto& v : secondTheirs.getVertices()) theirs.addVertex(v);
    ours.addFace({3, 4, 5});
    theirs.addFace({3, 4, 5});
    MergeCase merge({}, ours, theirs);
    REQUIRE(merge.session.getTotalConflicts() == 4);
    merge.session.prevConflict();
    REQUIRE(merge.session.getSelectedIndex() == 3);
    merge.session.nextConflict();
    REQUIRE(merge.session.getSelectedIndex() == 0);
    merge.session.prevConflict();
    merge.session.acceptTheirs();
    merge.session.autoMerge();
    REQUIRE(merge.session.getTotalConflicts() == 2);
    REQUIRE(merge.session.getSelectedIndex() == 1);
    merge.session.acceptOurs();
    merge.session.autoMerge();
    REQUIRE(merge.session.isResolved());
    REQUIRE(merge.session.getSelectedIndex() == 0);
    merge.session.acceptOurs();
    merge.session.acceptTheirs();
    merge.session.nextConflict();
    merge.session.prevConflict();
    REQUIRE(merge.session.isResolved());
}

TEST_CASE("Session validates null sources and invalid face indices", "[merge]") {
    const auto mesh = triangle();
    const std::vector<std::vector<std::size_t>> empty, bad{{0, 1, 99}};
    std::array<const std::vector<std::array<float, 3>>*, 3> vertices{&mesh.getVertices(), &mesh.getVertices(),
                                                                     &mesh.getVertices()};
    std::array<const std::vector<std::vector<std::size_t>>*, 6> faces{&empty, &empty, &empty, &empty, &empty, &empty};
    SECTION("null vertex source") { vertices[0] = nullptr; }
    SECTION("null faces") { faces[0] = nullptr; }
    SECTION("invalid vertex reference") { faces[1] = &bad; }
    REQUIRE_THROWS(ComparisonSession(vertices, faces, "merge"));
}
