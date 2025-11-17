#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <vector>

// ANSI color codes for furture enhancements
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string LIGHT_BLUE = "\033[94m";
const std::string PURPLE = "\033[35m";

const std::string COLOR_MATCH = RED; // color for matched pattern
const std::string COLOR_RESET = "\033[0m";   // reset color

// Function to colorize matched patterns in a line
std::string colorize_line(const std::string& line, const std::string& pattern, std::unordered_map<std::string, bool>& options) {

    size_t n = pattern.size();
    if (n == 0) return line; // avoid infinite loop on empty pattern

    size_t pos = 0;
    std::string result;

    // handle case insensitive search
    std::string lower_line = line;
    std::string lower_pattern = pattern;
    if (options["-i"] == true) {
        std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
        std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(), ::tolower);
    }

    while (true) {
        // Find the next match position
        size_t found = lower_line.find(lower_pattern, pos);

        if (found == std::string::npos) {
            // No more matches, append the remaining part
            result += line.substr(pos);
            break;
        }

        result += line.substr(pos, found - pos);
        result += COLOR_MATCH + line.substr(found, n) + COLOR_RESET; // add colored match

        pos = found + n;
    }
    return result;
}

// Function to extract all matching substrings from a line
void extract_matches(const std::string& line, const std::string& pattern, std::unordered_map<std::string, bool>& options, std::vector<std::string>& matches) {
    size_t n = pattern.size();
    if (n == 0) return;

    std::string lower_line = line;
    std::string lower_pattern = pattern;
    if (options["-i"]) {
        std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
        std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(), ::tolower);
    }

    size_t pos = 0;
    while (true) {
        size_t found = lower_line.find(lower_pattern, pos);
        if (found == std::string::npos) {
            break;
        }

        matches.push_back(line.substr(found, n));
        pos = found + n;
    }
}

// Main function
int main(int argc, char *argv[]) {
    // check for correct number of arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [options] <pattern> <filename...>" << std::endl;
        return 1;
    }

    // Options map for future enhancements
    std::unordered_map<std::string, bool> options{
        {"--help", false}, // Display help information
        {"-n", false}, // Number all output lines
        {"-i", false}, // Case insensitive search
        {"-v", false}, // Invert match
        {"-o", false}, // Only matching parts of lines
        {"-c", false} // Count of matching lines
    };

    // process options
    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg[0] == '-') { // options

            if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options] <pattern> <filename...>\n"
                          << "Options:\n"
                          << "  --help    Display this help information\n"
                          << "  -n        Number all output lines\n"
                          << "  -i        Case insensitive search\n"
                          << "  -v        Invert match\n"
                          << "  -o        Only matching parts of lines\n"
                          << "  -c        Count of matching lines\n";
                return 0;
            } else {

                // support merging of single-character options like -ni
                for (size_t j = 1; j < arg.size(); ++j) {
                    std::string opt("-" + std::string(1, arg[j]));

                    if (options.find(opt) != options.end()) {
                        options[opt] = true; // Enable the option
                    } else {
                        std::cerr << "Warning: Unknown option " << opt << std::endl;
                        return 1;
                    }
                }

                // Handle special case where -v and -o are both set without a search string
                if (options["-v"] == true && options["-o"] == true) {
                    return 0;
                }
            }
        } else { // string to grep
            break; // Stop processing options when the search string is encountered
        }
    }

    // Get the text information to be filtered
    std::string str = argv[i];
    if (str.empty()) {
        std::cerr << "Error: Search string cannot be empty." << std::endl;
        return 1;
    }
    ++i; // point to the first file

    std::string lower_str = str;
    if (options["-i"] == true) {
        // Convert search string to lowercase for case insensitive search
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    }

    // Determine if multiple files are being processed
    bool multiple_files = (argc - i > 1);

    // Determine if -c is used without -o
    // -o is prioritized over -c
    bool use_c = options["-c"] && !options["-o"];

    // process files
    for ( ; i != argc; ++i) {
        size_t line_number = 1; // counter for line number
        size_t match_count = 0; // counter for matching lines

        std::ifstream infile(argv[i]);
        if (!infile) { // check if file opened successfully
            std::cerr << "Error: Could not open file " << argv[i] << std::endl;
            continue; // proceed to the next file
        }

        std::string line;
        while (std::getline(infile, line)) {

            // handle case insensitive search
            std::string lower_line = line;
            if (options["-i"] == true) {
                std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
            }

            if (options["-v"] == true ? lower_line.find(lower_str) == std::string::npos : lower_line.find(lower_str) != std::string::npos) {

                if (use_c == false) {

                    if (options["-o"] == true) {
                        std::vector<std::string> matches;
                        extract_matches(line, str, options, matches);

                        for (const auto& match : matches) {

                            if (options["-n"] == true) {
                                std::cout << GREEN << line_number << COLOR_RESET 
                                        << LIGHT_BLUE << ": \t" << COLOR_RESET;
                            }
                            std::cout << COLOR_MATCH << match << COLOR_RESET << std::endl;
                        }
                    } else {

                        if (options["-n"] == true) {
                            std::cout << GREEN << line_number << COLOR_RESET 
                                    << LIGHT_BLUE << ": \t" << COLOR_RESET;
                        }
                        std::cout << colorize_line(line, str, options) << std::endl;
                    }
                } else {
                    
                    ++match_count;
                }
            }
             
            ++line_number;
        }
        infile.close();

        // Print count of matching lines if -c option is set
        if (use_c == true) {
            if (multiple_files == true) {
                std::cout << PURPLE << argv[i] << COLOR_RESET 
                          << LIGHT_BLUE << ":" << COLOR_RESET 
                          << match_count << std::endl;
            } else {
                std::cout << match_count << std::endl;
            }
        }
    }

    return 0;
}