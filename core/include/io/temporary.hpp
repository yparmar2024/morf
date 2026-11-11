#pragma once
#include <cerrno>
#include <filesystem>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <system_error>
#include <utility>

namespace Morf {
    // Own only a directory we created, never a path inferred from an input name.
    class TemporaryFile {
            std::filesystem::path directory_;

        public:
            explicit TemporaryFile(const std::filesystem::path& parent) {
                std::random_device random;
                for (int attempt = 0; attempt < 100; ++attempt) {
                    auto candidate = parent / (".morf-" + std::to_string(random()) + "-" + std::to_string(random()));
                    // Restrict access atomically, before any model data is written.
                    if (::mkdir(candidate.c_str(), 0700) == 0) {
                        directory_ = std::move(candidate);
                        return;
                    }
                    const int error = errno;
                    if (error != EEXIST)
                        throw std::filesystem::filesystem_error("Could not create temporary directory", candidate,
                                                                std::error_code(error, std::generic_category()));
                }
                throw std::runtime_error("Could not create a temporary directory in '" + parent.string() + "'.");
            }
            TemporaryFile(const TemporaryFile&) = delete;
            TemporaryFile& operator=(const TemporaryFile&) = delete;
            ~TemporaryFile() {
                std::error_code error;
                std::filesystem::remove_all(directory_, error);
            }
            std::filesystem::path path() const { return directory_ / "model.obj"; }
    };
} // namespace Morf
