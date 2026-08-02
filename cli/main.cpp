// morf - cli/main.cpp
#include "command/diff.hpp"
#include "command/merge.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> resolveGitLFS(const std::vector<std::string>& path);
int help(const std::string& subcommand);

int main(int argc, char* argv[]) {
    if (argc < 2) return help("help");

    std::vector<std::string> resolvedPaths;
    std::string subcommand = argv[1];
    if (subcommand == "-h" || subcommand == "--help" || subcommand == "help") {
        return help("help");
    } else if (subcommand == "diff") {
        if (argc == 4) {
            try {
                resolvedPaths = resolveGitLFS({ argv[2], argv[3] });
                Morf::Diff::run(resolvedPaths[0], resolvedPaths[1]);
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
                resolvedPaths = resolveGitLFS({ argv[2], argv[3], argv[4] });
                Morf::Merge::run(resolvedPaths[0], resolvedPaths[1], resolvedPaths[2]);
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

    for (const auto& path : resolvedPaths) {
        if (path.find(".smudged") != std::string::npos) std::remove(path.c_str());
    }

    return 0;
}

std::vector<std::string> resolveGitLFS(const std::vector<std::string>& paths) {
    std::vector<std::string> resolvedPaths;
    resolvedPaths.reserve(paths.size());

    for (const auto& path : paths) {
        std::ifstream file(path);
        if (!file.is_open()) {
            resolvedPaths.push_back(path);
            continue;
        }

        std::string firstLine;
        std::getline(file, firstLine);
        file.close();

        if (firstLine.find("version https://git-lfs.github.com/spec/v1") != std::string::npos) {
            std::string smudgedPath = path + ".smudged";

            std::string command = "git lfs smudge < \"" + path + "\" > \"" + smudgedPath + "\"";

            if (std::system(command.c_str()) == 0) {
                resolvedPaths.push_back(smudgedPath);
            } else {
                std::cerr << "Warning: Failed to expand LFS pointer for " << path << '\n';
                resolvedPaths.push_back(path);
            }
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
                  << "  diff    Compare models and compute differences\n"
                  << "  merge   Merge differences between models\n\n"
                  << "Options:\n"
                  << "  -h, --help    Show this help message\n";
        return 0;
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