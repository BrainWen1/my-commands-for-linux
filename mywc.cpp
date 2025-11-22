#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>

using std::cin;
using std::cout;
using std::cerr;
using std::string;
using std::unordered_map;
using std::vector;
using std::ifstream;
using std::istringstream;
using std::ostream;

ostream &process(std::istream &in, unordered_map<string, bool> &options);

int main(int argc, char *argv[]) {
    
    unordered_map<string, bool> options{
        {"--help", false}, // Display help information
        {"-l", false}, // Show line counts
        {"-c", false}, // Show byte counts
        {"-w", false} // Show word counts
    };

    // process command-line arguments
    vector<string> files;

    size_t i = 1;;
    for (i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg[0] == '-') { // options

            if (arg == "--help") {
                cout << "Usage: " << argv[0] << " [options] <filename...>\n"
                     << "Options:\n"
                     << "  --help    Display this help information\n"
                     << "  -l        Show line counts\n"
                     << "  -c        Show byte counts\n"
                     << "  -w        Show word counts\n";
                return 0;
            } else {

                for (size_t j = 1; j < arg.size(); ++j) {
                    string opt("-" + string(1, arg[j]));

                    if (options.find(opt) != options.end()) {
                        options[opt] = true; // Enable the option
                    } else {
                        cerr << "Warning: Unknown option " << opt << std::endl;
                        return 1;
                    }
                }
            }
        } else { // files
            files.push_back(arg);
        }
    }

    // process each file
    if (files.empty()) { // no files specified, read from standard input, support pipe input
        process(cin, options) << std::endl;
    } else { // files specified
        for (const auto &filename : files) {

            ifstream infile(filename);
            if (!infile) {
                cerr << "Error: Could not open file " << filename << std::endl;
                continue;
            }

            process(infile, options) << " " + filename << std::endl;
            infile.close();
        }
    }
}

ostream &process(std::istream &in, unordered_map<string, bool> &options) {
    size_t line_count = 0;
    size_t word_count = 0;
    size_t char_count = 0;

    string line;
    while (std::getline(in, line)) {
        ++line_count;
        char_count += line.size() + 1; // +1 for newline character

        istringstream iss(line);
        string word;
        while (iss >> word) {
            ++word_count;
        }
    }

    if (options["-l"] == true) {
        cout << "Lines: " << line_count << " ";
    }
    if (options["-w"] == true) {
        cout << "Words: " << word_count << " ";
    }
    if (options["-c"] == true) {
        cout << "Bytes: " << char_count << " ";
    }

    // default: show all counts if no specific option is given
    if (options["-l"] == false && options["-w"] == false && options["-c"] == false) {
        cout << "Lines: " << line_count << " "
             << "Words: " << word_count << " "
             << "Bytes: " << char_count << " ";
    }

    return cout;
}