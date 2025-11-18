#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>

namespace fs = std::filesystem;

const std::string GREEN = "\033[01;32m";
const std::string BLUE = "\033[01;34m";
const std::string CYAN = "\033[01;36m";
const std::string PURPLE = "\033[01;35m";
const std::string RESET = "\033[0m";   // reset

int main(int argc, char *argv[]) {
    
    // Options map for future enhancements
    std::unordered_map<std::string, bool> options{
        {"--help", false}, // Display help information
        {"-a", false} // Show all files including hidden files
    };

    // process options
    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg[0] == '-') { // options

            if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options] <directory...>\n"
                          << "Options:\n"
                          << "  --help    Display this help information\n"
                          << "  -a        Show all files including hidden files\n";
                return 0;
            } else {
                
                for (size_t j = 1; j < arg.size(); ++j) {
                    std::string opt("-" + std::string(1, arg[j]));

                    if (options.find(opt) != options.end()) {
                        options[opt] = true; // Enable the option
                    } else {
                        std::cerr << "Warning: Unknown option " << opt << std::endl;
                        return 1;
                    }
                }
            }
        } else { // directory path
            break; // Stop processing options when the directory path is encountered
        }
    }

    // Determine the starting index for directory paths
    fs::path dir_path = (argc > i) ? fs::path(argv[i]) : fs::current_path();
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cerr << "Error: " << dir_path << " is not a valid directory." << std::endl;
        return 1;
    }

    // Sort the list of entries
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        std::string name_a = a.path().filename().string();
        std::string name_b = b.path().filename().string();
        std::transform(name_a.begin(), name_a.end(), name_a.begin(), ::tolower);
        std::transform(name_b.begin(), name_b.end(), name_b.begin(), ::tolower);
        return name_a < name_b;
    });

    // List directory contents
    for (const auto& entry : entries) {
        std::string str = entry.path().filename().string();

        // skip hidden files unless -a is specified
        if (str[0] == '.' && options["-a"] == false) continue;

        if (entry.is_symlink()) { // symbolic link
            std::cout << CYAN << str << RESET;
        } else if (entry.is_directory()) { // directory
            std::cout << BLUE << str << RESET;
        } else { // file
            // Check executable permissions
            fs::perms perm = fs::status(entry).permissions();
            bool is_executable = (perm & fs::perms::owner_exec) != fs::perms::none
                    || (perm & fs::perms::group_exec) != fs::perms::none
                    || (perm & fs::perms::others_exec) != fs::perms::none;

            if (is_executable) { // executable file
                std::cout << GREEN << str << RESET ;
            } else { // regular file
                std::cout << str ;
            }
        }
        std::cout << "  ";
    }
    std::cout << std::endl;

    return 0;
}