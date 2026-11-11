#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace Morf {
    class ScopedPhase;

    // A single-thread phase report. Disabled sessions do not read the clock.
    class PhaseTimings {
        private:
            using Clock = std::chrono::steady_clock;
            static constexpr std::size_t none = std::numeric_limits<std::size_t>::max();
            struct Entry {
                    const char* name = nullptr;
                    std::size_t parent = none;
                    Clock::time_point start{};
                    double milliseconds = 0;
                    double childMilliseconds = 0;
                    bool finished = false;
                    bool failed = false;
            };

        public:
            class WorkerPhases {
                    friend class PhaseTimings;
                    std::vector<Entry> entries_;
            };

        private:
            struct Input {
                    std::string name;
                    std::uintmax_t bytes = 0;
                    bool sizeKnown = false;
            };
            inline static thread_local PhaseTimings* active_ = nullptr;
            PhaseTimings* previous_;
            bool enabled_;
            bool benchmark_;
            bool reported_ = false;
            bool failed_ = false;
            WorkerPhases* worker_ = nullptr;
            Clock::time_point start_{};
            std::size_t current_ = none;
            std::vector<Entry> entries_;
            std::vector<Input> inputs_;

            std::size_t begin(const char* name) {
                const auto index = entries_.size();
                entries_.push_back({name, current_, Clock::now()});
                current_ = index;
                return index;
            }
            void end(std::size_t index, bool failed) noexcept {
                auto& entry = entries_[index];
                entry.milliseconds = std::chrono::duration<double, std::milli>(Clock::now() - entry.start).count();
                entry.finished = true;
                entry.failed = failed;
                current_ = entry.parent;
                if (entry.parent != none) entries_[entry.parent].childMilliseconds += entry.milliseconds;
            }
            void report() noexcept {
                try {
                    print(std::cerr);
                } catch (...) {
                    // Optional diagnostics must not change command outcomes.
                }
            }
            friend class ScopedPhase;

        public:
            explicit PhaseTimings(bool enabled, bool benchmark = false)
                : previous_(active_), enabled_(enabled), benchmark_(benchmark) {
                if (enabled_) start_ = Clock::now();
                active_ = enabled_ ? this : nullptr;
            }
            PhaseTimings(bool enabled, WorkerPhases& worker) : PhaseTimings(enabled) { worker_ = &worker; }
            PhaseTimings(const PhaseTimings&) = delete;
            PhaseTimings& operator=(const PhaseTimings&) = delete;
            ~PhaseTimings() {
                active_ = previous_;
                if (worker_)
                    worker_->entries_ = std::move(entries_);
                else
                    report();
            }
            static bool isActive() noexcept { return active_ != nullptr; }
            // Read worker records only after the task has finished. Their roots overlap the loading phase.
            static void collectWorker(const WorkerPhases& worker) {
                if (!active_) return;
                auto& entries = active_->entries_;
                const auto offset = entries.size();
                for (auto entry : worker.entries_) {
                    if (entry.parent != none) entry.parent += offset;
                    entries.push_back(entry);
                }
            }
            void markFailed() noexcept { failed_ = true; }
            void addInput(const std::string& name, const std::string& resolvedPath) {
                if (!enabled_) return;
                std::error_code error;
                const auto size = std::filesystem::file_size(resolvedPath, error);
                inputs_.push_back({name, error ? 0 : size, !error});
            }
            static void reportStartup() noexcept {
                if (active_) active_->report();
            }
            void print(std::ostream& stream) {
                if (!enabled_ || reported_) return;
                const auto wall = std::chrono::duration<double, std::milli>(Clock::now() - start_).count();
                bool incomplete = failed_;
                for (const auto& entry : entries_) incomplete = incomplete || entry.failed || !entry.finished;
                std::ostringstream output;
                output << "\nmorf phase timings (single run; milliseconds)\n";
#ifdef MORF_BUILD_TYPE
                output << "Build: " << MORF_BUILD_TYPE << '\n';
#endif
                output << "Mode: " << (benchmark_ ? "headless benchmark; viewer and saving skipped" : "startup")
                       << '\n';
                output << "Status: " << (incomplete ? "incomplete/failed" : "complete") << '\n';
                for (const auto& input : inputs_) {
                    output << "Input: " << input.name;
                    if (input.sizeKnown) output << " (" << input.bytes << " bytes)";
                    output << '\n';
                }
                output << std::left << std::setw(44) << "Phase" << std::right << std::setw(14) << "Inclusive ms"
                       << std::setw(14) << "Exclusive ms" << '\n';
                output << std::fixed << std::setprecision(3);
                for (const auto& entry : entries_) {
                    std::size_t depth = 0;
                    for (auto parent = entry.parent; parent != none; parent = entries_[parent].parent) ++depth;
                    std::string label(depth * 2, ' ');
                    label += entry.name;
                    if (entry.failed)
                        label += " [failed]";
                    else if (!entry.finished)
                        label += " [active]";
                    output << std::left << std::setw(44) << label << std::right << std::setw(14) << entry.milliseconds
                           << std::setw(14) << entry.milliseconds - entry.childMilliseconds << '\n';
                }
                output << "Startup/processing wall: " << wall << " ms (includes uninstrumented work).\n";
                output << "Nested inclusive values overlap; do not sum them.\n";
                output << "Model workers may overlap each other and the OBJ loading wall time.\n";
                stream << output.str();
                reported_ = true;
            }
    };

    class ScopedPhase {
        private:
            PhaseTimings* timings_;
            std::size_t index_ = 0;
            int exceptions_ = 0;

        public:
            // Phase names must outlive the report; instrumentation uses string literals.
            explicit ScopedPhase(const char* name) : timings_(PhaseTimings::active_) {
                if (!timings_) return;
                exceptions_ = std::uncaught_exceptions();
                index_ = timings_->begin(name);
            }
            ScopedPhase(const ScopedPhase&) = delete;
            ScopedPhase& operator=(const ScopedPhase&) = delete;
            ~ScopedPhase() { stop(); }
            void stop() noexcept {
                if (!timings_) return;
                timings_->end(index_, std::uncaught_exceptions() > exceptions_);
                timings_ = nullptr;
            }
    };
} // namespace Morf
