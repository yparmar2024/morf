/* morf - gui/include/application/session.hpp
 * Session logic to store current model states
 */
#pragma once
#include "raylib.h"
#include <array>
#include <cstddef>
#include <vector>
#include <utility>

namespace Morf {
    class Session {
        private:
            const std::vector<std::array<float, 3>>& baseVertices_;
            const std::vector<std::array<float, 3>>& leftVertices_;
            const std::vector<std::array<float, 3>>& rightVertices_;

            std::vector<std::vector<std::size_t>> inactiveBaseIndices_;
            std::vector<std::vector<std::size_t>> inactiveLeftIndices_;
            std::vector<std::vector<std::size_t>> inactiveRightIndices_;

            std::vector<std::vector<std::size_t>> activeLeftAdds_;
            std::vector<std::vector<std::size_t>> activeLeftRemoves_;
            std::vector<std::vector<std::size_t>> activeRightAdds_;
            std::vector<std::vector<std::size_t>> activeRightRemoves_;

            const std::string mode_;
            int selected_ = 0;

        public:
            Session(
                const std::array<const std::vector<std::array<float, 3>>*, 3>& verticePtrs,
                const std::array<const std::vector<std::vector<std::size_t>>*, 6>& indicePtrs,
                const std::string& mode
            );
            void autoMerge();

            int getTotalConflicts() const;
            void nextConflict();
            void prevConflict();
            void acceptOurs();
            void acceptTheirs();

            bool isResolved() const {
                return (mode_ == "diff" ||
                (mode_ == "merge" && activeLeftAdds_.empty() && activeLeftRemoves_.empty() && activeRightAdds_.empty() && activeRightRemoves_.empty()))
                ? true : false;
            }
            const std::string& getMode() const { return mode_; }
            int getSelectedIndex() const { return selected_; }

            const std::vector<std::array<float, 3>>& getBaseVertices() const { return baseVertices_; }
            const std::vector<std::array<float, 3>>& getOurVertices() const { return leftVertices_; }
            const std::vector<std::array<float, 3>>& getTheirVertices() const { return rightVertices_; }

            const std::vector<std::vector<std::size_t>>& getCommonBaseIndices() const { return inactiveBaseIndices_; }
            const std::vector<std::vector<std::size_t>>& getOurBaseIndices() const { return inactiveLeftIndices_; }
            const std::vector<std::vector<std::size_t>>& getTheirBaseIndices() const { return inactiveRightIndices_; }

            const std::vector<std::vector<std::size_t>>& getOurAddedIndices() const { return activeLeftAdds_; }
            const std::vector<std::vector<std::size_t>>& getOurRemovedIndices() const { return activeLeftRemoves_; }
            const std::vector<std::vector<std::size_t>>& getTheirAddedIndices() const { return activeRightAdds_; }
            const std::vector<std::vector<std::size_t>>& getTheirRemovedIndices() const { return activeRightRemoves_; }

            const std::pair<std::vector<std::array<float, 3>>, std::vector<std::vector<std::size_t>>> getMergedGeometry() const;
    };
} // namespace Morf