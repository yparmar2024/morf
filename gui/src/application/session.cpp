// morf - gui/src/application/application.cpp
#include "application/session.hpp"
#include "geometry/bounds.hpp"
#include <algorithm>
#include <iterator>
#include <map>
#include <set>

namespace Morf {
    Session::Session(
        const std::array<const std::vector<std::array<float, 3>>*, 3>& verticePtrs,
        const std::array<const std::vector<std::vector<std::size_t>>*, 6>& indicePtrs,
        const std::string& mode
    ) :
        baseVertices_(*(verticePtrs[0])),
        leftVertices_(*(verticePtrs[1])),
        rightVertices_(*(verticePtrs[2])),
        mode_(mode)
    {
        if (mode_ == "diff") {
            inactiveBaseIndices_  = *(indicePtrs[3]);
            inactiveLeftIndices_  = {};
            inactiveRightIndices_ = {};

            activeLeftAdds_       = {};
            activeLeftRemoves_    = {};
            activeRightAdds_      = *(indicePtrs[4]);
            activeRightRemoves_   = *(indicePtrs[5]);
        } else if (mode_ == "merge") {
            auto isLess = [&](const std::vector<std::size_t>& a, const std::vector<std::size_t>& b) -> bool
            {
                if (a.size() != b.size()) return a.size() < b.size();

                for (std::size_t i = 0; i < a.size(); ++i) {
                    if (baseVertices_[a[i]] < baseVertices_[b[i]]) return true;
                    if (baseVertices_[a[i]] > baseVertices_[b[i]]) return false;
                }
                return false;
            };

            std::set_intersection(
                (*indicePtrs[0]).begin(), (*indicePtrs[0]).end(),
                (*indicePtrs[3]).begin(), (*indicePtrs[3]).end(),
                std::back_inserter(inactiveBaseIndices_),
                isLess
            );
            inactiveLeftIndices_  = {};
            inactiveRightIndices_ = {};

            activeLeftAdds_       = *(indicePtrs[1]);
            activeLeftRemoves_    = *(indicePtrs[2]);
            activeRightAdds_      = *(indicePtrs[4]);
            activeRightRemoves_   = *(indicePtrs[5]);

            autoMerge();
        }
    }

    void Session::autoMerge() {
        const auto isLess = [&](const std::vector<std::size_t>& baseFace, const std::vector<std::array<float, 3>>& baseVertices,
                                const std::vector<std::size_t>& targetFace, const std::vector<std::array<float, 3>>& targetVertices) -> bool
        {
            if (baseFace.size() != targetFace.size()) return baseFace.size() < targetFace.size();

            for (std::size_t i = 0; i < baseFace.size(); ++i) {
                if (baseVertices[baseFace[i]] < targetVertices[targetFace[i]]) return true;
                if (baseVertices[baseFace[i]] > targetVertices[targetFace[i]]) return false;
            }
            return false;
        };

        std::vector<std::vector<std::size_t>> remainingLeftAdds;
        std::vector<std::vector<std::size_t>> remainingRightAdds;

        std::size_t i = 0, j = 0;
        while (i < activeLeftAdds_.size() && j < activeRightAdds_.size()) {
            const auto& leftFace = activeLeftAdds_[i];
            const auto& rightFace = activeRightAdds_[j];

            if (isLess(leftFace, leftVertices_, rightFace, rightVertices_)) {
                remainingLeftAdds.push_back(leftFace);
                i++;
            } else if (isLess(rightFace, rightVertices_, leftFace, leftVertices_)) {
                remainingRightAdds.push_back(rightFace);
                j++;
            } else {
                inactiveLeftIndices_.push_back(leftFace);
                i++;
                j++;
            }
        }
        while (i < activeLeftAdds_.size()) remainingLeftAdds.push_back(activeLeftAdds_[i++]);
        while (j < activeRightAdds_.size()) remainingRightAdds.push_back(activeRightAdds_[j++]);

        std::vector<std::vector<std::size_t>> remainingLeftRemoves;
        std::vector<std::vector<std::size_t>> remainingRightRemoves;

        i = 0, j = 0;
        while (i < activeLeftRemoves_.size() && j < activeRightRemoves_.size()) {
            const auto& leftFace = activeLeftRemoves_[i];
            const auto& rightFace = activeRightRemoves_[j];

            if (isLess(leftFace, baseVertices_, rightFace, baseVertices_)) {
                remainingLeftRemoves.push_back(leftFace);
                i++;
            } else if (isLess(rightFace, baseVertices_, leftFace, baseVertices_)) {
                remainingRightRemoves.push_back(rightFace);
                j++;
            } else {
                i++;
                j++;
            }
        }
        while (i < activeLeftRemoves_.size()) remainingLeftRemoves.push_back(activeLeftRemoves_[i++]);
        while (j < activeRightRemoves_.size()) remainingRightRemoves.push_back(activeRightRemoves_[j++]);

        // Merge conflicts only occur between the following:
        // - left adds to right adds
        // - left adds to right removes
        // - left removes to right adds
        std::vector<bool> leftAddCollides(remainingLeftAdds.size(), false);
        std::vector<bool> rightAddCollides(remainingRightAdds.size(), false);
        std::vector<bool> leftRemoveCollides(remainingLeftRemoves.size(), false);
        std::vector<bool> rightRemoveCollides(remainingRightRemoves.size(), false);

        for (std::size_t i = 0; i < remainingLeftAdds.size(); ++i) {
            AABB lBox = getFaceBox(remainingLeftAdds[i], leftVertices_);

            for (std::size_t j = 0; j < remainingRightAdds.size(); ++j) {
                if (boxesOverlap(lBox, getFaceBox(remainingRightAdds[j], rightVertices_))) {
                    leftAddCollides[i] = true;
                    rightAddCollides[j] = true;
                }
            }

            for (std::size_t j = 0; j < remainingRightRemoves.size(); ++j) {
                if (boxesOverlap(lBox, getFaceBox(remainingRightRemoves[j], baseVertices_))) {
                    leftAddCollides[i] = true;
                    rightRemoveCollides[j] = true;
                }
            }
        }

        for (std::size_t i = 0; i < remainingLeftRemoves.size(); ++i) {
            AABB lBox = getFaceBox(remainingLeftRemoves[i], baseVertices_);

            for (std::size_t j = 0; j < remainingRightAdds.size(); ++j) {
                if (boxesOverlap(lBox, getFaceBox(remainingRightAdds[j], rightVertices_))) {
                    leftRemoveCollides[i] = true;
                    rightAddCollides[j] = true;
                }
            }
        }

        activeLeftAdds_.clear();
        activeRightAdds_.clear();
        activeLeftRemoves_.clear();
        activeRightRemoves_.clear();

        for (std::size_t i = 0; i < remainingLeftAdds.size(); ++i) {
            if (leftAddCollides[i]) activeLeftAdds_.push_back(remainingLeftAdds[i]);
            else inactiveLeftIndices_.push_back(remainingLeftAdds[i]);
        }
        for (std::size_t i = 0; i < remainingRightAdds.size(); ++i) {
            if (rightAddCollides[i]) activeRightAdds_.push_back(remainingRightAdds[i]);
            else inactiveRightIndices_.push_back(remainingRightAdds[i]);
        }
        for (std::size_t i = 0; i < remainingLeftRemoves.size(); ++i) {
            if (leftRemoveCollides[i]) activeLeftRemoves_.push_back(remainingLeftRemoves[i]);
        }
        for (std::size_t i = 0; i < remainingRightRemoves.size(); ++i) {
            if (rightRemoveCollides[i]) activeRightRemoves_.push_back(remainingRightRemoves[i]);
        }
    }

    int Session::getTotalConflicts() const {
        return activeLeftAdds_.size() + activeLeftRemoves_.size() +
               activeRightAdds_.size() + activeRightRemoves_.size();
    };

    void Session::nextConflict() {
        int total = getTotalConflicts();
        if (total > 0) selected_ = (selected_ + 1) % total;
    };

    void Session::prevConflict() {
        int total = getTotalConflicts();
        if (total > 0) selected_ = (selected_ - 1 + total) % total;
    };

    void Session::acceptOurs() {
        if (getTotalConflicts() == 0) return;
        int idx = selected_;

        if (idx < activeLeftAdds_.size()) {
            inactiveLeftIndices_.push_back(activeLeftAdds_[idx]);
            activeLeftAdds_.erase(activeLeftAdds_.begin() + idx);
        } else if ((idx -= activeLeftAdds_.size()) < activeLeftRemoves_.size()) {
            activeLeftRemoves_.erase(activeLeftRemoves_.begin() + idx);
        } else if ((idx -= activeLeftRemoves_.size()) < activeRightAdds_.size()) {
            activeRightAdds_.erase(activeRightAdds_.begin() + idx);
        } else {
            idx -= activeRightAdds_.size();
            inactiveBaseIndices_.push_back(activeRightRemoves_[idx]);
            activeRightRemoves_.erase(activeRightRemoves_.begin() + idx);
        }
        if (getTotalConflicts() > 0 && selected_ >= getTotalConflicts()) selected_ = getTotalConflicts() - 1;
    }

    void Session::acceptTheirs() {
        if (getTotalConflicts() == 0) return;
        int idx = selected_;

        if (idx < activeLeftAdds_.size()) {
            activeLeftAdds_.erase(activeLeftAdds_.begin() + idx);
        } else if ((idx -= activeLeftAdds_.size()) < activeLeftRemoves_.size()) {
            inactiveBaseIndices_.push_back(activeLeftRemoves_[idx]);
            activeLeftRemoves_.erase(activeLeftRemoves_.begin() + idx);
        } else if ((idx -= activeLeftRemoves_.size()) < activeRightAdds_.size()) {
            inactiveRightIndices_.push_back(activeRightAdds_[idx]);
            activeRightAdds_.erase(activeRightAdds_.begin() + idx);
        } else {
            idx -= activeRightAdds_.size();
            activeRightRemoves_.erase(activeRightRemoves_.begin() + idx);
        }
        if (getTotalConflicts() > 0 && selected_ >= getTotalConflicts()) selected_ = getTotalConflicts() - 1;
    }

    const std::pair<std::vector<std::array<float, 3>>, std::vector<std::vector<std::size_t>>> Session::getMergedGeometry() const {
        std::vector<std::array<float, 3>> mergedVertices;
        std::vector<std::vector<std::size_t>> mergedIndices;
        std::map<std::array<float, 3>, std::size_t> vertexMap;
        std::set<std::vector<std::size_t>> uniqueFaces;

        auto addFace = [&](const std::vector<std::size_t>& faceIndices, const std::vector<std::array<float, 3>>& sourceVertices) {
            std::vector<std::size_t> newFaceIndices;
            for (std::size_t vIdx : faceIndices) {
                const auto& pt = sourceVertices[vIdx];

                if (vertexMap.find(pt) == vertexMap.end()) {
                    vertexMap[pt] = mergedVertices.size();
                    mergedVertices.push_back(pt);
                }
                newFaceIndices.push_back(vertexMap[pt]);
            }

            if (uniqueFaces.insert(newFaceIndices).second) {
                mergedIndices.push_back(newFaceIndices);
            }
        };

        for (const auto& f : inactiveBaseIndices_)  addFace(f, baseVertices_);
        for (const auto& f : inactiveLeftIndices_)  addFace(f, leftVertices_);
        for (const auto& f : inactiveRightIndices_) addFace(f, rightVertices_);

        return { mergedVertices, mergedIndices }; 
    }
} // namespace Morf