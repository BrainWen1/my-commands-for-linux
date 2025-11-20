#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

string get_dir_name(dev_t target_dev, ino_t target_ino) {

    // open current directory
    DIR *dir = opendir(".");
    if (dir == nullptr) {
        perror("opendir error 4");
        exit(4);
    }

    struct dirent *entry; // directory entry pointer
    struct stat curr;
    string dir_name;

    while ((entry = readdir(dir)) != nullptr) {
        // get stat for each entry
        if (lstat(entry->d_name, &curr) == -1) {
            if (errno == EACCES) {
                continue; // skip entries we can't access
            } else {
                perror("lstat error 5");
                closedir(dir);
                exit(5);
            }
        }

        // check if this entry matches the target
        if (curr.st_dev == target_dev && curr.st_ino == target_ino) {
            dir_name = entry->d_name; // found the directory name
            break;
        }
    }
    closedir(dir); // avoid resource leak

    return dir_name;
}

int main(void) {
    vector<string> path_units; // store directory names
    struct stat current_stat, parent_stat; // store stat info for current and parent directories

    // get stat for current directory
    if (lstat(".", &current_stat) == -1) { // check for errors
        perror("lstat error 1");
        exit(1);
    }

    // loop until reaching root directory
    while (true) {
        // get stat for parent directory
        if (lstat("..", &parent_stat) == -1) {
            perror("lstat error 2");
            exit(2);
        }

        // check if current directory is root
        if (current_stat.st_dev == parent_stat.st_dev &&
            current_stat.st_ino == parent_stat.st_ino) {
            break; // reached root directory
        }

        // change to parent directory
        if (chdir("..") == -1) {
            perror("chdir error 3");
            exit(3);
        }
        // get directory name from parent directory
        string dir_name = get_dir_name(current_stat.st_dev, current_stat.st_ino);
        path_units.push_back(dir_name); // store directory name

        current_stat = parent_stat; // update current_stat to parent_stat
    }

    // print the absolute path
    cout << "/";
    for (auto it = path_units.rbegin(); it != path_units.rend(); ++it) {
        cout << *it;

        if (it + 1 != path_units.rend()) {
            cout << "/";
        }
    }
    cout << endl;

    return 0;
}
