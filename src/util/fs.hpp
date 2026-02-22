#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace iris::util::fs {

// path operations
std::string join(const std::string& base, const std::string& path);
std::string normalize(const std::string& path);
std::string absolute(const std::string& path);
std::string relative(const std::string& path, const std::string& base = "");
std::string dirname(const std::string& path);
std::string basename(const std::string& path);
std::string stem(const std::string& path);
std::string extension(const std::string& path);

// file operations
bool exists(const std::string& path);
bool is_file(const std::string& path);
bool is_directory(const std::string& path);
bool is_symlink(const std::string& path);

std::string read_file(const std::string& path);
bool write_file(const std::string& path, const std::string& content);
bool append_file(const std::string& path, const std::string& content);

bool copy_file(const std::string& src, const std::string& dst, bool overwrite = false);
bool move_file(const std::string& src, const std::string& dst);
bool remove_file(const std::string& path);

// directory operations
bool create_directory(const std::string& path);
bool create_directories(const std::string& path);
bool remove_directory(const std::string& path);
bool remove_all(const std::string& path);

std::vector<std::string> list_directory(const std::string& path);
std::vector<std::string> list_files(const std::string& path, bool recursive = false);
std::vector<std::string> list_directories(const std::string& path);

// glob and pattern matching
std::vector<std::string> glob(const std::string& pattern);
bool matches_glob(const std::string& path, const std::string& pattern);

// file info
uint64_t file_size(const std::string& path);
int64_t modification_time(const std::string& path);
bool is_newer(const std::string& path1, const std::string& path2);

// temporary files
std::string temp_directory();
std::string create_temp_file(const std::string& prefix = "iris_");
std::string create_temp_directory(const std::string& prefix = "iris_");

// walking
using WalkCallback = std::function<void(const std::string& path, bool is_dir)>;
void walk(const std::string& path, WalkCallback callback, bool follow_symlinks = false);

// current directory
std::string current_path();
bool set_current_path(const std::string& path);

} // namespace iris::util::fs
