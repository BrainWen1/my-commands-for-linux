#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <pwd.h> // getpwuid
#include <grp.h> // getgrgid
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sys/stat.h> // stat

namespace fs = std::filesystem;

const std::string GREEN = "\033[01;32m";
const std::string BLUE = "\033[01;34m";
const std::string CYAN = "\033[01;36m";
const std::string PURPLE = "\033[01;35m";
const std::string RESET = "\033[0m";   // reset

struct LongFormatInfo {
    std::string permissions; // 权限字符串（如"drwxr-xr-x"）
    uintmax_t link_count;    // 硬链接数
    std::string owner;       // 所有者用户名
    std::string group;       // 组名
    uintmax_t size;          // 文件大小（字节）
    std::string mtime;       // 最后修改时间（"月 日 时:分"）
    std::string name;        // 文件名（带颜色）
    std::string symlink_target; // 软链接目标（非软链接为空）
};

std::string get_permissions_string(const fs::directory_entry& entry);
std::string get_username(uid_t uid);
std::string get_groupname(gid_t gid);
std::string get_mtime_string(const fs::directory_entry& entry);
void collect_long_info(const std::vector<fs::directory_entry>& entries,
                      std::vector<LongFormatInfo>& long_entries, std::unordered_map<std::string, bool>& options);
void print_long_format(const std::vector<LongFormatInfo>& long_entries);

int main(int argc, char *argv[]) {
    
    // Options map for future enhancements
    std::unordered_map<std::string, bool> options{
        {"--help", false}, // Display help information
        {"-a", false}, // Show all files including hidden files
        {"-l", false} // Long format listing
    };

    // process options
    size_t i = 1;
    for (i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg[0] == '-') { // options

            if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options] <directory...>\n"
                          << "Options:\n"
                          << "  --help    Display this help information\n"
                          << "  -a        Show all files including hidden files\n"
                          << "  -l        Long format listing\n";
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
        } else { // directory path
            break; // Stop processing options when the directory path is encountered
        }
    }

    // Determine the starting index for directory paths
    fs::path dir_path = (argc > i) ? fs::path(argv[i]) : fs::current_path();
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cerr << "Error: " << dir_path << " is not a valid directory." << std::endl;
        return 1;
    }

    // Sort the list of entries
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        std::string name_a = a.path().filename().string();
        std::string name_b = b.path().filename().string();
        std::transform(name_a.begin(), name_a.end(), name_a.begin(), ::tolower);
        std::transform(name_b.begin(), name_b.end(), name_b.begin(), ::tolower);
        return name_a < name_b;
    });

    // List directory contents
    if (options["-l"] == true) { // -l option for long format

        std::vector<LongFormatInfo> long_entries;
        collect_long_info(entries, long_entries, options);
        print_long_format(long_entries);

    } else { // default format

        for (const auto& entry : entries) {
            std::string str = entry.path().filename().string();

            // skip hidden files unless -a is specified
            if (str[0] == '.' && options["-a"] == false) continue;

            if (entry.is_symlink()) { // symbolic link
                std::cout << CYAN << str << RESET;
            } else if (entry.is_directory()) { // directory
                std::cout << BLUE << str << RESET;
            } else { // file
                // Check executable permissions
                fs::perms perm = fs::status(entry).permissions();
                bool is_executable = (perm & fs::perms::owner_exec) != fs::perms::none
                        || (perm & fs::perms::group_exec) != fs::perms::none
                        || (perm & fs::perms::others_exec) != fs::perms::none;

                if (is_executable) { // executable file
                    std::cout << GREEN << str << RESET ;
                } else { // regular file
                    std::cout << str ;
                }
            }
            std::cout << "  ";
        }
        std::cout << std::endl;
    }

    return 0;
}

std::string get_permissions_string(const fs::directory_entry& entry) {
    std::string perm_str(10, '-'); // 10位：类型+9权限位

    // 第0位：文件类型
    if (entry.is_directory()) perm_str[0] = 'd';
    else if (entry.is_symlink()) perm_str[0] = 'l';
    else if (entry.is_regular_file()) perm_str[0] = '-';
    else perm_str[0] = '?';

    // 获取权限位
    fs::perms p;
    if (entry.is_symlink()) {
        p = fs::symlink_status(entry).permissions(); // 软链接自身权限
    } else {
        p = fs::status(entry).permissions(); // 非软链接用目标权限
    }

    // 所有者权限（1-3位）
    perm_str[1] = ((p & fs::perms::owner_read) != fs::perms::none) ? 'r' : '-';
    perm_str[2] = ((p & fs::perms::owner_write) != fs::perms::none) ? 'w' : '-';
    perm_str[3] = ((p & fs::perms::owner_exec) != fs::perms::none) ? 'x' : '-';

    // 组权限（4-6位）
    perm_str[4] = ((p & fs::perms::group_read) != fs::perms::none) ? 'r' : '-';
    perm_str[5] = ((p & fs::perms::group_write) != fs::perms::none) ? 'w' : '-';
    perm_str[6] = ((p & fs::perms::group_exec) != fs::perms::none) ? 'x' : '-';

    // 其他用户权限（7-9位）
    perm_str[7] = ((p & fs::perms::others_read) != fs::perms::none) ? 'r' : '-';
    perm_str[8] = ((p & fs::perms::others_write) != fs::perms::none) ? 'w' : '-';
    perm_str[9] = ((p & fs::perms::others_exec) != fs::perms::none) ? 'x' : '-';

    return perm_str;
}

std::string get_username(uid_t uid) {
    struct passwd* pw = getpwuid(uid);
    return (pw != nullptr) ? pw->pw_name : std::to_string(uid);
}

// 通过组ID获取组名
std::string get_groupname(gid_t gid) {
    struct group* gr = getgrgid(gid);
    return (gr != nullptr) ? gr->gr_name : std::to_string(gid);
}

std::string get_mtime_string(const fs::directory_entry& entry) {
    auto ftime = fs::last_write_time(entry);
    using namespace std::chrono;
    auto sys_time = system_clock::to_time_t(
        system_clock::time_point(duration_cast<system_clock::duration>(ftime.time_since_epoch()))
    );
    struct tm* local_tm = localtime(&sys_time);
    if (local_tm == nullptr) { // 处理转换失败
        return "Invalid time";
    }
    char buf[20];
    strftime(buf, sizeof(buf), "%b %d %H:%M", local_tm);
    return buf;
}

void collect_long_info(const std::vector<fs::directory_entry>& entries,
                      std::vector<LongFormatInfo>& long_entries, std::unordered_map<std::string, bool>& options) {
    for (const auto& entry : entries) {
        std::string name = entry.path().filename().string();
        if (name[0] == '.' && !options["-a"]) continue; // 结合-a选项

        // 新增：用stat获取文件元信息（包括uid和gid）
        struct stat file_stat;
        if (stat(entry.path().c_str(), &file_stat) != 0) {
            // 处理获取失败的情况（如权限不足）
            long_entries.push_back({"...", 0, "?", "?", 0, "", name, ""});
            continue;
        }

        LongFormatInfo info;
        info.permissions = get_permissions_string(entry);
        info.link_count = file_stat.st_nlink;  // 用stat的硬链接数
        info.owner = get_username(file_stat.st_uid);  // 从stat获取uid
        info.group = get_groupname(file_stat.st_gid); // 从stat获取gid
        info.size = file_stat.st_size;
        info.mtime = get_mtime_string(entry);

        // 文件名颜色逻辑不变...
        if (entry.is_symlink()) {
            info.name = CYAN + name + RESET;
            info.symlink_target = fs::read_symlink(entry.path()).string();
        } else if (entry.is_directory()) {
            info.name = BLUE + name + RESET;
        } else {
            fs::perms perm = fs::status(entry).permissions();
            bool is_exec = (perm & fs::perms::owner_exec) != fs::perms::none;
            info.name = is_exec ? (GREEN + name + RESET) : name;
        }

        long_entries.push_back(info);
    }
}

void print_long_format(const std::vector<LongFormatInfo>& long_entries) {
    for (const auto& info : long_entries) {
        // 按列对齐输出（用setw控制宽度）
        std::cout << std::left
                  << std::setw(11) << info.permissions  // 权限（10位+空格）
                  << std::right
                  << info.link_count    // 链接数
                  << " " << std::left
                  << std::setw(4) << info.owner         // 所有者
                  << " "
                  << std::setw(4) << info.group         // 组
                  << " " << std::setw(8) << info.size          // 大小
                  << std::setw(12) << info.mtime        // 修改时间
                  << " " << info.name;                         // 文件名

        // 软链接额外显示" -> 目标"
        if (!info.symlink_target.empty()) {
            std::cout << " -> " << info.symlink_target;
        }
        std::cout << std::endl;
    }
}