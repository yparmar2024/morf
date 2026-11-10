#pragma once
#include "formats/obj/handler.hpp"

#include <array>
#include <cstddef>
#include <future>
#include <iostream>
#include <sstream>
#include <string>

namespace Morf {
    template <std::size_t Count>
    std::array<GenericModel, Count> loadObjModels(const std::array<std::string, Count>& paths) {
        // Futures wait for their workers on exit, so diagnostics must outlive them.
        std::array<std::ostringstream, Count> diagnostics;
        std::array<std::future<GenericModel>, Count> loads;
        for (std::size_t i = 0; i < Count; ++i) {
            loads[i] = std::async(std::launch::async, [&paths, &diagnostics, i] {
                return Handler::convert(Handler::parse(paths[i], diagnostics[i]));
            });
        }

        // Report warnings and propagate errors in input order.
        std::array<GenericModel, Count> models;
        for (std::size_t i = 0; i < Count; ++i) {
            models[i] = loads[i].get();
            const auto warnings = diagnostics[i].str();
            if (!warnings.empty()) std::cerr << warnings;
        }
        return models;
    }
} // namespace Morf
