// morf - cli/main.cpp
#include "command/diff.hpp"
#include "command/init.hpp"
#include "command/merge.hpp"
#include "io/process.hpp"
#include "io/temporary.hpp"

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

    ResolvedInputs inputs;
    std::vector<std::string> resolvedPaths;
    int result = 0;
    std::string subcommand = argv[1];
    if (subcommand == "-h" || subcommand == "--help" || subcommand == "help") {
        return help("help");
    } else if (subcommand == "init") {
        if (argc == 2) {
            try {
                result = Morf::Init::run();
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << '\n';
                return 1;
            }
        } else {
            return help("init");
        }
    } else if (subcommand == "diff") {
        if (argc == 4) {
            try {
                resolvedPaths = inputs.resolve({argv[2], argv[3]});
                result = Morf::Diff::run(resolvedPaths[0], resolvedPaths[1]);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << '\n';
                return 1;
            }
        } else {
            return help("diff");
        }
    } else if (subcommand == "merge") {
        if (argc == 5) {
            try {
                resolvedPaths = inputs.resolve({argv[2], argv[3], argv[4]});
                result = Morf::Merge::run(resolvedPaths[0], resolvedPaths[1], resolvedPaths[2], argv[3]);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << '\n';
                return 1;
            }
        } else {
            return help("merge");
        }
    } else {
        return help(subcommand);
    }

    return result;
}

std::vector<std::string> ResolvedInputs::resolve(const std::vector<std::string>& paths) {
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
                  << "  -h, --help    Show this help message\n";
        return 0;
    } else if (subcommand == "init") {
        std::cerr << "Usage: morf init\n\n"
                  << "Example:\n"
                  << "  morf init\n";
        return 1;
    } else if (subcommand == "diff") {
        std::cerr << "Usage: morf diff <baseFile> <targetFile>\n\n"
                  << "Example:\n"
                  << "  morf diff old.obj new.obj\n";
        return 1;
    } else if (subcommand == "merge") {
        std::cerr << "Usage: morf merge <originalFile> <ourFile> <theirFile>\n\n"
                  << "Example:\n"
                  << "  morf merge base.obj ours.obj theirs.obj\n";
        return 1;
    } else {
        std::cerr << "morf: '" << subcommand << "' is not a morf command. See 'morf --help'\n";
        return 1;
    }
}