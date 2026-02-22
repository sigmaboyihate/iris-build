#pragma once

#include <string>
#include <chrono>

namespace iris::ui {

class BuildProgress {
public:
    void start();
    void compile(const std::string& file, int current, int total);
    void link(const std::string& target);
    void finish(bool success, int compiled, int failed);
    
private:
    std::chrono::steady_clock::time_point m_start;
    
    std::string short_path(const std::string& path);
};

} // namespace iris::ui
