#pragma once
#include "formats/obj/handler.hpp"
#include "performance/timing.hpp"

#include <array>
#include <cstddef>
#include <future>
#include <iostream>
#include <sstream>
#include <string>

namespace Morf {
    template <std::size_t Count>
    std::array<GenericModel, Count> loadObjModels(const std::array<std::string, Count>& paths,
                                                  const std::array<const char*, Count>& labels) {
        // Futures wait for their workers on exit, so the worker outputs must outlive them.
        std::array<std::ostringstream, Count> diagnostics;
        std::array<PhaseTimings::WorkerPhases, Count> workerPhases;
        const bool timingEnabled = PhaseTimings::isActive();
        ScopedPhase loading("Load OBJ inputs (wall)");
        std::array<std::future<GenericModel>, Count> loads;
        for (std::size_t i = 0; i < Count; ++i) {
            loads[i] = std::async(std::launch::async,
                                  [&paths, &diagnostics, &workerPhases, timingEnabled, label = labels[i], i] {
                                      PhaseTimings timings(timingEnabled, workerPhases[i]);
                                      ScopedPhase phase(label);
                                      return Handler::convert(Handler::parse(paths[i], diagnostics[i]));
                                  });
        }

        // Report warnings and propagate errors in input order.
        std::array<GenericModel, Count> models;
        for (std::size_t i = 0; i < Count; ++i) {
            if (timingEnabled) {
                loads[i].wait();
                PhaseTimings::collectWorker(workerPhases[i]);
            }
            models[i] = loads[i].get();
            const auto warnings = diagnostics[i].str();
            if (!warnings.empty()) std::cerr << warnings;
        }
        return models;
    }
} // namespace Morf
