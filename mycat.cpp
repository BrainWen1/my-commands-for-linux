#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2 ... fileN]" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc ; ++i) {
        std::ifstream infile(argv[i]);
        if (!infile) {
            std::cerr << "Error: Could not open file " << argv[i] << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(infile, line)) {
            std::cout << line << std::endl;
        }
        infile.close();

        if (argc > 2 && i < argc - 1) {
            std::cout << std::endl; // Print a newline between files
        }
    }
    return 0;
}