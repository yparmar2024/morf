/* morf - core/include/formats/obj/handler.hpp
 * Handler logic for .obj file format
 */
#pragma once
#include "formats/model.hpp"
#include "formats/obj/model.hpp"
#include <string>

namespace Morf {
    class Handler {
        public:
            static Model parse(const std::string& filePath);
            static GenericModel convert(const Model& model);
            static void write(const std::string& filePath, const GenericModel& model);
    };
} // namespace Morf