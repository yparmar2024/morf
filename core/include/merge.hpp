/* morf - core/include/merge.hpp
 * Merging logic to convert Diff to a generalized Merge struct to translate to Raylib
 */
#pragma once
#include "diff.hpp"
#include <vector>

namespace Morf {
    struct Merge {
        std::vector<FaceRef> common;
        std::vector<FaceRef> added;
        std::vector<FaceRef> removed;

        std::vector<FaceRef> acceptedFromBase;
        std::vector<FaceRef> acceptedFromTarget;
        
        int selectedIndex = 0;
        
        bool hasPending() const { return !added.empty() || !removed.empty(); }
        int totalChanges() const { return added.size() + removed.size(); }
    };

    class MergeEngine {
        public:
            static Merge create(const Diff& diff) {
                Merge merge;
                merge.common  = diff.common;
                merge.added   = diff.added;
                merge.removed = diff.removed;
                return merge;
            }

            static void accept(Merge& merge) {
                if (merge.selectedIndex < merge.added.size()) {
                    merge.acceptedFromTarget.push_back(merge.added[merge.selectedIndex]);
                    merge.added.erase(merge.added.begin() + merge.selectedIndex);
                } else {
                    merge.removed.erase(merge.removed.begin() + (merge.selectedIndex - merge.added.size()));
                }
                clampIndex(merge);
            }

            static void reject(Merge& merge) {
                if (merge.selectedIndex < merge.added.size()) {
                    merge.added.erase(merge.added.begin() + merge.selectedIndex);
                } else {
                    merge.acceptedFromBase.push_back(merge.removed[merge.selectedIndex - merge.added.size()]);
                    merge.removed.erase(merge.removed.begin() + (merge.selectedIndex - merge.added.size()));
                }
                clampIndex(merge);
            }

        private:
            static void clampIndex(Merge& merge) {
                int total = merge.totalChanges();
                if (total == 0) merge.selectedIndex = 0;
                else if (merge.selectedIndex >= total) merge.selectedIndex = total - 1;
            }
    };
}