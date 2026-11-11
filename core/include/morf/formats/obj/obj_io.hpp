/* morf - core/include/morf/formats/obj/obj_io.hpp
 * Read, convert, and write OBJ models.
 */
#pragma once
#include "morf/formats/obj/obj_model.hpp"
#include "morf/geometry/mesh.hpp"

#include <iosfwd>
#include <string>

namespace Morf {
    class ObjIO {
        public:
            static ObjModel parse(const std::string& filePath);
            static ObjModel parse(const std::string& filePath, std::ostream& diagnostics);
            static PolygonMesh convert(const ObjModel& model);
            static void write(const std::string& filePath, const PolygonMesh& model);
    };
} // namespace Morf
