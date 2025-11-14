#include <iostream>
#include <fstream>
#include <string>
#include <map>

int main(int argc, char *argv[]) {
    // Check for at least one file argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2 ... fileN]" << std::endl;
        return 1;
    }

    // Options map for future enhancements
    std::map<std::string, bool> options{
        {"-n", false}, // Number all output lines
        {"--help", false} // Display help information
    };

    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg[0] == '-') { // options

            if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options] <file1> [file2 ... fileN]\n"
                          << "Options:\n"
                          << "  -n        Number all output lines\n"
                          << "  --help    Display this help information\n";
                return 0;

            } else if (options.find(arg) != options.end()) {
                options[arg] = true;

            } else {
                std::cerr << "Unknown option: " << arg << std::endl;
                return 1;

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

        std::string line;
        while (std::getline(infile, line)) {

            // Handle options here in the future

            if (options["-n"] == true) {
                std::cout << line_number++ << "\t";
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