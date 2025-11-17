#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

int main(int argc, char *argv[]) {
    // Check for at least one file argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [options] <filename...>" << std::endl;
        return 1;
    }

    // Options map for future enhancements
    std::unordered_map<std::string, bool> options{
        {"--help", false}, // Display help information
        {"-n", false}, // Number all output lines
        {"-b", false}, // Number non-blank output lines
        {"-s", false} // Squeeze multiple adjacent blank lines
    };

    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg[0] == '-') { // options

            if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options] <filename...>\n"
                          << "Options:\n"
                          << "  --help    Display this help information\n"
                          << "  -n        Number all output lines\n"
                          << "  -b        Number non-blank output lines\n"
                          << "  -s        Squeeze multiple adjacent blank lines\n";
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
        } else { // files
            break; // Stop processing options when a file is encountered
        }
    }

    for ( ; i < argc ; ++i) { // Process each file from the remaining arguments
        size_t line_number = 1; // counter for line number

        std::ifstream infile(argv[i]);
        if (!infile) {
            std::cerr << "Error: Could not open file " << argv[i] << std::endl;
            continue;
        }

        // sign for squeezing blank lines
        bool squeeze_blank_lines = false;

        std::string line;
        while (std::getline(infile, line)) {

            // Handle options here in the future

            if (options["-s"] == true) {
                if (line.empty()) {
                    if (squeeze_blank_lines == true) {
                        continue; // Skip this line
                    } else {
                        squeeze_blank_lines = true; // Mark that we've seen a blank line
                    }
                } else {
                    squeeze_blank_lines = false; // Reset on non-blank line
                }
            }

            if (options["-n"] == true || options["-b"] == true) {
                if (options["-b"] == true && line.empty()) {
                    std::cout << "\t"; // No line number for blank lines
                } else {
                    std::cout << line_number++ << "\t";
                }
            }

            // ending options handling

            std::cout << line << std::endl;
        }
        infile.close();

        // Print a newline between files if there are multiple files
        if (argc > 2 && i < argc - 1) {
            std::cout << std::endl;
        }
    }
    return 0;
}