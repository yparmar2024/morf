// morf - cli/main.cpp
#include "command/diff.hpp"
#include "command/merge.hpp"
#include <iostream>
#include <string>

int help(const std::string& subcommand);

int main(int argc, char* argv[]) {
    if (argc < 2) return help("help");

    std::string subcommand = argv[1];
    if (subcommand == "-h" || subcommand == "--help" || subcommand == "help") {
        return help("help");
    } else if (subcommand == "diff") {
        if (argc == 4) {
            try {
                Morf::Diff::run(argv[2], argv[3]);
                return 0;
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
                Morf::Merge::run(argv[2], argv[3], argv[4]);
                return 0;
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

    return 0;
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