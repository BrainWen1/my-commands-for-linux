#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using std::string;
using std::cerr;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename...>\n";
        return 1; // No file specified
    }


    // Open the file, create it if it doesn't exist, and set permissions to rw-r--r--
    for (int i = 1; i < argc; ++i) {
        
        const char* filename = argv[i];
        int fd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (fd == -1) {
            cerr << "Error creating file: " << filename << "\n";
            return 1; // Error creating file
        }
        close(fd);
    }

    return 0; // Success
}