#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

using std::cerr;
using std::unordered_map;
using std::string;
using std::vector;

void recursive_remove(const std::string& path, bool &force, bool &recursive);

int main(int argc, char* argv[]) {
    // check for at least one filename argument
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " [options] <filename...>\n";
        return 1;
    }

    // Options map for future enhancements
    unordered_map<std::string, bool> options{
        {"--help", false},  // display help message
        {"-f", false}, // force delete
        {"-r", false}  // recursive delete
    };

    vector<string> files;

    // Process command-line arguments
    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg[0] == '-') { // options

            if (arg == "--help") { // help option
                std::cout << "Usage: " << argv[0] << " [options] <filename...>\n"
                          << "Options:\n"
                          << "  --help    Display this help information\n"
                          << "  -f        Force delete\n"
                          << "  -r        Recursive delete\n";
                return 0;

            } else { // other options

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
        } else { // files
            files.push_back(arg);
        }
    }

    bool force = options["-f"];
    bool recursive = options["-r"];

    // Process each file
    for (const auto& file : files) {
        recursive_remove(file, force, recursive);
    }

    return 0;
}

void recursive_remove(const string& path, bool &force, bool &recursive) {

    struct stat st;

    // get file status
    if (stat(path.c_str(), &st) == -1) {
        if (force == false || errno != ENOENT) {
            cerr << "Error: Failed to access '" << path << "': " << strerror(errno) << "\n";
        }
        return;
    }

    // target is a regular file → delete directly
    if (S_ISREG(st.st_mode)) {
        if (remove(path.c_str()) != 0) {
            if (force == false) { // only report error if not in force mode
                cerr << "Error: Failed to delete '" << path << "': " << strerror(errno) << "\n";
            }
        }
        return;
    }

    // target is a directory → require -r for recursive delete
    if (S_ISDIR(st.st_mode)) {
        if (recursive == false) { // -r not enabled → error
            cerr << "Error: '" << path << "' is a directory. Use -r to delete recursively.\n";
            return;
        }

        // -r enabled → open directory and iterate entries
        DIR* dir = opendir(path.c_str());
        if (dir == nullptr) {
            if (force == false) {
                cerr << "Error: Failed to open directory '" << path << "': " << strerror(errno) << "\n";
            }
            return;
        }

        struct dirent* entry;
        // iterate over directory entries
        while ((entry = readdir(dir)) != nullptr) {
            string sub_name = entry->d_name;
            // skip . and .. to avoid infinite recursion
            if (sub_name == "." || sub_name == "..") {
                continue;
            }
            // concatenate full path of sub-item: path/sub-item
            string sub_path = path + "/" + sub_name;
            
            recursive_remove(sub_path, force, recursive); // recursive delete sub-item
        }
        closedir(dir); // close directory

        // delete empty directory (rmdir can only delete empty directories)
        if (rmdir(path.c_str()) != 0) {
            if (force == false) {
                cerr << "Error: Failed to delete directory '" << path << "': " << strerror(errno) << "\n";
            }
        }
        return;
    }

    // other file types (e.g., symbolic links, devices) → delete directly
    if (remove(path.c_str()) != 0) {
        if (force == false) {
            cerr << "Error: Failed to delete '" << path << "': " << strerror(errno) << "\n";
        }
    }
}
