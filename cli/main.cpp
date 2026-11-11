// morf - cli/main.cpp
#include "command/diff.hpp"
#include "command/init.hpp"
#include "command/merge.hpp"
#include "io/process.hpp"
#include "morf/io/temporary_file.hpp"
#include "morf/performance/timing.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class ResolvedInputs {
        std::vector<std::unique_ptr<Morf::TemporaryFile>> temporaryFiles_;

    public:
        std::vector<std::string> resolve(const std::vector<std::string>& paths);
};
int help(const std::string& subcommand);

int main(int argc, char* argv[]) {
    if (argc < 2) return help("help");

    const std::string subcommand = argv[1];
    if (subcommand == "-h" || subcommand == "--help" || subcommand == "help") {
        return help("help");
    }
    if (subcommand != "init" && subcommand != "diff" && subcommand != "merge") {
        return help(subcommand);
    }

    bool showTimings = false;
    bool benchmark = false;
    bool literalPaths = false;
    std::vector<std::string> paths;
    for (int i = 2; i < argc; ++i) {
        const std::string argument = argv[i];
        if (!literalPaths && argument == "--")
            literalPaths = true;
        else if (!literalPaths && argument == "--timings")
            showTimings = true;
        else if (!literalPaths && argument == "--benchmark") {
            showTimings = true;
            benchmark = true;
        } else if (!literalPaths && (argument == "--help" || argument == "-h")) {
            help(subcommand);
            return 0;
        } else if (!literalPaths && argument.rfind("--", 0) == 0) {
            std::cerr << "Error: Unknown option '" << argument << "'.\n";
            return help(subcommand);
        } else
            paths.push_back(argument);
    }
    const std::size_t expectedPaths = subcommand == "init" ? 0 : subcommand == "diff" ? 2 : 3;
    if (paths.size() != expectedPaths || (subcommand == "init" && benchmark)) return help(subcommand);

    Morf::PhaseTimings timings(showTimings, benchmark);
    ResolvedInputs inputs;
    try {
        if (subcommand == "init") {
            Morf::ScopedPhase phase("Configure Git integration");
            return Morf::Init::run();
        }
        const auto resolvedPaths = inputs.resolve(paths);
        for (std::size_t i = 0; i < paths.size(); ++i) timings.addInput(paths[i], resolvedPaths[i]);
        if (subcommand == "diff") return Morf::Diff::run(resolvedPaths[0], resolvedPaths[1], benchmark);
        return Morf::Merge::run(resolvedPaths[0], resolvedPaths[1], resolvedPaths[2], paths[1], benchmark);
    } catch (const std::exception& e) {
        timings.markFailed();
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}

std::vector<std::string> ResolvedInputs::resolve(const std::vector<std::string>& paths) {
    Morf::ScopedPhase phase("Resolve inputs / Git LFS");
    std::vector<std::string> resolvedPaths;
    resolvedPaths.reserve(paths.size());
    for (const auto& path : paths) {
        std::ifstream file(path);
        if (!file.is_open()) throw std::runtime_error("Failed to open '" + path + "' for reading.");
        std::string firstLine;
        std::getline(file, firstLine);
        if (file.bad()) throw std::runtime_error("Failed while reading '" + path + "'.");
        if (!firstLine.empty() && firstLine.back() == '\r') firstLine.pop_back();
        file.close();
        if (firstLine == "version https://git-lfs.github.com/spec/v1") {
            auto temporary = std::make_unique<Morf::TemporaryFile>(std::filesystem::temp_directory_path());
            const std::string expanded = temporary->path().string();
            Morf::runCommand("git lfs smudge < " + Morf::shellQuote(path) + " > " + Morf::shellQuote(expanded));
            // A skipped smudge filter may report success while returning the pointer unchanged.
            std::ifstream output(expanded);
            std::string header;
            std::getline(output, header);
            if (!output.is_open() || output.bad() || header.find("version https://git-lfs.github.com/spec/v1") == 0)
                throw std::runtime_error("Failed to expand LFS pointer for '" + path + "'.");
            resolvedPaths.push_back(expanded);
            temporaryFiles_.push_back(std::move(temporary));
        } else {
            resolvedPaths.push_back(path);
        }
    }
    return resolvedPaths;
}

int help(const std::string& subcommand) {
    if (subcommand == "help") {
        std::cout << "Usage: morf <command> [options]\n\n"
                  << "Commands:\n"
                  << "  help    Show this help message\n"
                  << "  init    Initialize the current repository\n"
                  << "  diff    Compare models and compute differences\n"
                  << "  merge   Merge differences between models\n\n"
                  << "Options:\n"
                  << "  -h, --help    Show this help message\n"
                  << "  --timings     Report processing phases (available in Release builds)\n"
                  << "  --benchmark   Diff/merge processing only; no viewer or merge output\n"
                  << "  --            Treat remaining arguments as file paths\n";
        return 0;
    } else if (subcommand == "init") {
        std::cerr << "Usage: morf init [--timings]\n\n"
                  << "Example:\n"
                  << "  morf init\n";
        return 1;
    } else if (subcommand == "diff") {
        std::cerr << "Usage: morf diff [--timings | --benchmark] <baseFile> <targetFile>\n\n"
                  << "Example:\n"
                  << "  morf diff old.obj new.obj\n";
        return 1;
    } else if (subcommand == "merge") {
        std::cerr << "Usage: morf merge [--timings | --benchmark] <originalFile> <ourFile> <theirFile>\n\n"
                  << "Example:\n"
                  << "  morf merge base.obj ours.obj theirs.obj\n";
        return 1;
    } else {
        std::cerr << "morf: '" << subcommand << "' is not a morf command. See 'morf --help'\n";
        return 1;
    }
}
