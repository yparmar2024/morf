/* morf - core/include/compare/engine.hpp
 * Engine for computing deltas
 */
#pragma once
#include "compare/delta.hpp"
#include "formats/model.hpp"

namespace Morf {
    class Engine {
        public:
            static Delta computeDelta(const GenericModel& base, const GenericModel& target);
    };
} // namespace Morf