#include "progress.hpp"
#include "terminal.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace iris::ui {

void BuildProgress::start() {
    m_start = std::chrono::steady_clock::now();
}

void BuildProgress::compile(const std::string& file, int current, int total) {
    std::cout << "\r\033[K";
    
    Terminal::print_styled("  [", Color::Gray);
    std::cout << current << "/" << total;
    Terminal::print_styled("] ", Color::Gray);
    
    // detect file type from extension
    std::string ext;
    size_t dot = file.rfind('.');
    if (dot != std::string::npos) {
        ext = file.substr(dot);
    }
    bool is_cpp = (ext == ".cpp" || ext == ".cxx" || ext == ".cc" || ext == ".C");
    Terminal::print_styled(is_cpp ? "CXX " : "CC  ", Color::Cyan);
    std::cout << short_path(file) << std::flush;
}

void BuildProgress::link(const std::string& target) {
    std::cout << "\r\033[K";
    Terminal::print_styled("  LINK ", Color::Magenta);
    std::cout << target << std::flush;
}

void BuildProgress::finish(bool success, int compiled, int failed) {
    std::cout << "\r\033[K";
    
    auto end = std::chrono::steady_clock::now();
    double secs = std::chrono::duration<double>(end - m_start).count();
    
    std::cout << "\n";
    
    if (success) {
        Terminal::print_styled("  ✓ ", Color::Green, Style::Bold);
        std::cout << "Built " << compiled << " file" << (compiled != 1 ? "s" : "");
    } else {
        Terminal::print_styled("  ✗ ", Color::Red, Style::Bold);
        std::cout << "Build failed";
        if (failed > 0) {
            std::cout << " (" << failed << " error" << (failed != 1 ? "s" : "") << ")";
        }
    }
    
    Terminal::print_styled(" [", Color::Gray);
    std::cout << std::fixed << std::setprecision(2) << secs << "s";
    Terminal::print_styled("]\n", Color::Gray);
}

std::string BuildProgress::short_path(const std::string& path) {
    //rRemove ../ prefix and shorten if needed
    std::string p = path;
    
    while (p.substr(0, 3) == "../") {
        p = p.substr(3);
    }
    
    if (p.length() > 50) {
        size_t slash = p.rfind('/');
        if (slash != std::string::npos && slash > 10) {
            p = "..." + p.substr(slash);
        }
    }
    
    return p;
}

} // namespace iris::ui
