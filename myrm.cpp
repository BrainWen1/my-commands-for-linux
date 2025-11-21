#include <iostream>

using std::cerr;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename...>\n";
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        const char* filename = argv[i];
        
        if (remove(filename) != 0) {
            cerr << "Error deleting file: " << filename << "\n";
            return 1;
        }
    }

    return 0;
}