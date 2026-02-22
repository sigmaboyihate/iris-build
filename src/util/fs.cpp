#include "fs.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <random>

namespace stdfs = std::filesystem;

namespace iris::util::fs {

std::string join(const std::string& base, const std::string& path) {
    return (stdfs::path(base) / path).string();
}

std::string normalize(const std::string& path) {
    return stdfs::path(path).lexically_normal().string();
}

std::string absolute(const std::string& path) {
    return stdfs::absolute(path).string();
}

std::string relative(const std::string& path, const std::string& base) {
    if (base.empty()) {
        return stdfs::relative(path).string();
    }
    return stdfs::relative(path, base).string();
}

std::string dirname(const std::string& path) {
    return stdfs::path(path).parent_path().string();
}

std::string basename(const std::string& path) {
    return stdfs::path(path).filename().string();
}

std::string stem(const std::string& path) {
    return stdfs::path(path).stem().string();
}

std::string extension(const std::string& path) {
    return stdfs::path(path).extension().string();
}

bool exists(const std::string& path) {
    return stdfs::exists(path);
}

bool is_file(const std::string& path) {
    return stdfs::is_regular_file(path);
}

bool is_directory(const std::string& path) {
    return stdfs::is_directory(path);
}

bool is_symlink(const std::string& path) {
    return stdfs::is_symlink(path);
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool write_file(const std::string& path, const std::string& content) {
    // create parent directories if needed
    stdfs::path p(path);
    if (p.has_parent_path()) {
        stdfs::create_directories(p.parent_path());
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

bool append_file(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::app | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

bool copy_file(const std::string& src, const std::string& dst, bool overwrite) {
    try {
        auto options = overwrite ? 
            stdfs::copy_options::overwrite_existing : 
            stdfs::copy_options::none;
        stdfs::copy_file(src, dst, options);
        return true;
    } catch (...) {
        return false;
    }
}

bool move_file(const std::string& src, const std::string& dst) {
    try {
        stdfs::rename(src, dst);
        return true;
    } catch (...) {
        return false;
    }
}

bool remove_file(const std::string& path) {
    try {
        return stdfs::remove(path);
    } catch (...) {
        return false;
    }
}

bool create_directory(const std::string& path) {
    try {
        return stdfs::create_directory(path);
    } catch (...) {
        return false;
    }
}

bool create_directories(const std::string& path) {
    try {
        return stdfs::create_directories(path);
    } catch (...) {
        return false;
    }
}

bool remove_directory(const std::string& path) {
    try {
        return stdfs::remove(path);
    } catch (...) {
        return false;
    }
}

bool remove_all(const std::string& path) {
    try {
        stdfs::remove_all(path);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> list_directory(const std::string& path) {
    std::vector<std::string> result;
    
    if (!stdfs::exists(path) || !stdfs::is_directory(path)) {
        return result;
    }
    
    for (const auto& entry : stdfs::directory_iterator(path)) {
        result.push_back(entry.path().filename().string());
    }
    
    return result;
}

std::vector<std::string> list_files(const std::string& path, bool recursive) {
    std::vector<std::string> result;
    
    if (!stdfs::exists(path) || !stdfs::is_directory(path)) {
        return result;
    }
    
    if (recursive) {
        for (const auto& entry : stdfs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                result.push_back(entry.path().string());
            }
        }
    } else {
        for (const auto& entry : stdfs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                result.push_back(entry.path().string());
            }
        }
    }
    
    return result;
}

std::vector<std::string> list_directories(const std::string& path) {
    std::vector<std::string> result;
    
    if (!stdfs::exists(path) || !stdfs::is_directory(path)) {
        return result;
    }
    
    for (const auto& entry : stdfs::directory_iterator(path)) {
        if (entry.is_directory()) {
            result.push_back(entry.path().string());
        }
    }
    
    return result;
}

std::vector<std::string> glob(const std::string& pattern) {
    std::vector<std::string> result;
    
    stdfs::path p(pattern);
    stdfs::path base_path = p.parent_path();
    std::string glob_pattern = p.filename().string();
    
    if (base_path.empty()) {
        base_path = ".";
    }
    
    // convert glob to regex
    std::string regex_pattern;
    for (char c : glob_pattern) {
        switch (c) {
            case '*':
                regex_pattern += ".*";
                break;
            case '?':
                regex_pattern += ".";
                break;
            case '.':
                regex_pattern += "\\.";
                break;
            case '[':
            case ']':
            case '(':
            case ')':
            case '{':
            case '}':
            case '+':
            case '^':
            case '$':
            case '|':
            case '\\':
                regex_pattern += '\\';
                regex_pattern += c;
                break;
            default:
                regex_pattern += c;
        }
    }
    
    std::regex re(regex_pattern);
    
    if (stdfs::exists(base_path)) {
        bool recursive = (glob_pattern.find("**") != std::string::npos);
        
        if (recursive) {
            for (const auto& entry : stdfs::recursive_directory_iterator(base_path)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, re)) {
                        result.push_back(entry.path().string());
                    }
                }
            }
        } else {
            for (const auto& entry : stdfs::directory_iterator(base_path)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, re)) {
                        result.push_back(entry.path().string());
                    }
                }
            }
        }
    }
    
    return result;
}

bool matches_glob(const std::string& path, const std::string& pattern) {
    // convert glob to regex (again)
    std::string regex_pattern;
    for (char c : pattern) {
        switch (c) {
            case '*':
                regex_pattern += ".*";
                break;
            case '?':
                regex_pattern += ".";
                break;
            case '.':
                regex_pattern += "\\.";
                break;
            default:
                regex_pattern += c;
        }
    }
    
    std::regex re(regex_pattern);
    return std::regex_match(path, re);
}

uint64_t file_size(const std::string& path) {
    try {
        return stdfs::file_size(path);
    } catch (...) {
        return 0;
    }
}

int64_t modification_time(const std::string& path) {
    try {
        auto ftime = stdfs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::seconds>(
            ftime - stdfs::file_time_type::clock::now() + 
            std::chrono::system_clock::now()
        );
        return sctp.time_since_epoch().count();
    } catch (...) {
        return 0;
    }
}

bool is_newer(const std::string& path1, const std::string& path2) {
    try {
        return stdfs::last_write_time(path1) > stdfs::last_write_time(path2);
    } catch (...) {
        return false;
    }
}

std::string temp_directory() {
    return stdfs::temp_directory_path().string();
}

std::string create_temp_file(const std::string& prefix) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    
    stdfs::path temp_dir = stdfs::temp_directory_path();
    std::string filename = prefix + std::to_string(dis(gen));
    
    stdfs::path temp_path = temp_dir / filename;
    
    std::ofstream file(temp_path);
    file.close();
    
    return temp_path.string();
}

std::string create_temp_directory(const std::string& prefix) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    
    stdfs::path temp_dir = stdfs::temp_directory_path();
    std::string dirname = prefix + std::to_string(dis(gen));
    
    stdfs::path temp_path = temp_dir / dirname;
    stdfs::create_directories(temp_path);
    
    return temp_path.string();
}

void walk(const std::string& path, WalkCallback callback, bool follow_symlinks) {
    if (!stdfs::exists(path)) return;
    
    auto options = follow_symlinks ? 
        stdfs::directory_options::follow_directory_symlink :
        stdfs::directory_options::none;
    
    for (const auto& entry : stdfs::recursive_directory_iterator(path, options)) {
        callback(entry.path().string(), entry.is_directory());
    }
}

std::string current_path() {
    return stdfs::current_path().string();
}

bool set_current_path(const std::string& path) {
    try {
        stdfs::current_path(path);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace iris::util::fs
