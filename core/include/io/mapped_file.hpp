#pragma once
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace Morf {
    // Read-only POSIX mapping. The input must remain unchanged while being read.
    class MappedFile {
        private:
            void* mapping_ = nullptr;
            std::size_t size_ = 0;
            std::size_t position_ = 0;

        public:
            explicit MappedFile(const std::string& filePath) {
                // Nonblocking open prevents unsupported inputs such as FIFOs from hanging.
                const int descriptor = ::open(filePath.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK);
                if (descriptor < 0) {
                    const int error = errno;
                    throw std::system_error(error, std::generic_category(), "Failed to open '" + filePath + "'");
                }

                struct stat status{};
                if (::fstat(descriptor, &status) != 0) {
                    const int error = errno;
                    ::close(descriptor);
                    throw std::system_error(error, std::generic_category(), "Failed to inspect '" + filePath + "'");
                }
                if (!S_ISREG(status.st_mode) || status.st_size < 0 ||
                    static_cast<std::uintmax_t>(status.st_size) >
                        static_cast<std::uintmax_t>(std::numeric_limits<std::ptrdiff_t>::max())) {
                    ::close(descriptor);
                    throw std::runtime_error("Input must be a regular file with a mappable size: '" + filePath + "'.");
                }
                if (status.st_size == 0) {
                    ::close(descriptor);
                    return;
                }

                const auto size = static_cast<std::size_t>(status.st_size);
                void* mapping = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, descriptor, 0);
                const int error = mapping == MAP_FAILED ? errno : 0;
                ::close(descriptor);
                if (mapping == MAP_FAILED)
                    throw std::system_error(error, std::generic_category(), "Failed to map '" + filePath + "'");
                mapping_ = mapping;
                size_ = size;
            }

            MappedFile(const MappedFile&) = delete;
            MappedFile& operator=(const MappedFile&) = delete;
            ~MappedFile() {
                if (size_ != 0) ::munmap(mapping_, size_);
            }

            // Views remain valid until this mapping is destroyed. Match getline's newline handling.
            bool readLine(std::string_view& line) {
                line = {};
                if (position_ == size_) return false;
                const auto* begin = static_cast<const char*>(mapping_) + position_;
                const auto* newline = static_cast<const char*>(std::memchr(begin, '\n', size_ - position_));
                const auto length = newline ? static_cast<std::size_t>(newline - begin) : size_ - position_;
                line = std::string_view(begin, length);
                position_ += length + (newline ? 1 : 0);
                return true;
            }
    };
} // namespace Morf
