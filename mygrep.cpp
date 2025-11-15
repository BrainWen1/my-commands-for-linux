#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

int main(int argc, char *argv[]) {
    // check for correct number of arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // Options map for future enhancements
    std::unordered_map<std::string, bool> options{
        {"--help", false}, // Display help information
        {"-n", false} // Number all output lines
    };

    // process options
    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg[0] == '-') { // options

            if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options] <filename>\n"
                          << "Options:\n"
                          << "  --help    Display this help information\n"
                          << "  -n        Number all output lines\n";
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
        } else { // string to grep
            break; // Stop processing options when the search string is encountered
        }
    }

    // Get the text information to be filtered
    std::string str = argv[i];
    ++i; // point to the first file

    // process files
    for ( ; i != argc; ++i) {
        size_t line_number = 1; // counter for line number

        std::ifstream infile(argv[i]);
        if (!infile) {
            std::cerr << "Error: Could not open file " << argv[i] << std::endl;
            continue; // proceed to the next file
        }

        std::string line;
        while (std::getline(infile, line)) {

            if (line.find(str) != std::string::npos) { // match found

                if (options["-n"] == true) {
                    std::cout << line_number++ << "\t";
                }

                std::cout << line << std::endl;
            }
        }
        infile.close();
    }

    return 0;
}