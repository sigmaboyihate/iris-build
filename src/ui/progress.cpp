#include "progress.hpp"
#include "terminal.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace iris::ui {

const std::vector<std::string> Progress::SPINNER_FRAMES = {
    "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
};

const std::vector<std::string> Progress::PROGRESS_CHARS = {
    "░", "▒", "▓", "█"
};

Progress::Progress() = default;

Progress::~Progress() {
    if (m_running) {
        finish();
    }
}

void Progress::start(const std::string& task_name) {
    m_task_name = task_name;
    m_current = 0;
    m_total = 0;
    m_running = true;
    m_start_time = std::chrono::steady_clock::now();
    
    // Start spinner thread
    m_spinner_thread = std::thread(&Progress::spinner_loop, this);
}

void Progress::update(const std::string& task_name, int current, int total) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current_message = task_name;
    m_current = current;
    m_total = total;
}

void Progress::update(double percentage) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current = static_cast<int>(percentage);
    m_total = 100;
}

void Progress::set_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current_message = message;
}

void Progress::increment() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current++;
}

void Progress::set_total(int total) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_total = total;
}

void Progress::finish() {
    m_running = false;
    
    if (m_spinner_thread.joinable()) {
        m_spinner_thread.join();
    }
    
    Terminal::clear_line();
    Terminal::show_cursor();
}

void Progress::fail(const std::string& error) {
    m_running = false;
    
    if (m_spinner_thread.joinable()) {
        m_spinner_thread.join();
    }
    
    Terminal::clear_line();
    Terminal::show_cursor();
    
    std::cout << "  ";
    Terminal::print_styled("✗", Color::Red, Style::Bold);
    std::cout << " " << m_task_name;
    if (!error.empty()) {
        std::cout << ": " << error;
    }
    std::cout << std::endl;
}

void Progress::spinner_loop() {
    Terminal::hide_cursor();
    
    while (m_running) {
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        m_spinner_index = (m_spinner_index + 1) % SPINNER_FRAMES.size();
    }
    
    Terminal::show_cursor();
}

void Progress::render() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Terminal::clear_line();
    
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - m_start_time).count();
    
    std::cout << "  ";
    Terminal::print_styled(SPINNER_FRAMES[m_spinner_index], Color::Cyan, Style::Bold);
    std::cout << " ";
    Terminal::print_styled(m_task_name, Color::White, Style::Bold);
    
    if (m_total > 0) {
        double percentage = (static_cast<double>(m_current) / m_total) * 100.0;
        
        std::cout << " ";
        Terminal::print_styled("[", Color::Gray);
        
        int bar_width = 20;
        int filled = static_cast<int>((percentage / 100.0) * bar_width);
        
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) {
                Terminal::print_styled("█", Color::Cyan);
            } else if (i == filled) {
                Terminal::print_styled("▓", Color::Cyan);
            } else {
                Terminal::print_styled("░", Color::Gray);
            }
        }
        
        Terminal::print_styled("]", Color::Gray);
        
        std::cout << " ";
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << percentage << "%";
        Terminal::print_styled(oss.str(), Color::White);
        
        std::cout << " ";
        Terminal::print_styled("(" + std::to_string(m_current) + "/" + 
                               std::to_string(m_total) + ")", Color::Gray);
    }
    
    if (!m_current_message.empty()) {
        std::cout << " ";
        Terminal::print_styled(m_current_message, Color::Gray);
    }
    
    std::cout << " ";
    Terminal::print_styled(format_duration(elapsed), Color::Gray);
    
    std::cout << std::flush;
}

std::string Progress::format_duration(double seconds) const {
    std::ostringstream oss;
    
    if (seconds < 60) {
        oss << std::fixed << std::setprecision(1) << seconds << "s";
    } else if (seconds < 3600) {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        oss << mins << "m " << secs << "s";
    } else {
        int hours = static_cast<int>(seconds) / 3600;
        int mins = (static_cast<int>(seconds) % 3600) / 60;
        oss << hours << "h " << mins << "m";
    }
    
    return oss.str();
}

std::string Progress::get_spinner_frame() const {
    return SPINNER_FRAMES[m_spinner_index];
}

// ProgressBar implementation

ProgressBar::ProgressBar(int total, int width) 
    : m_total(total), m_width(width) {}

void ProgressBar::update(int current) {
    m_current = current;
    render();
}

void ProgressBar::increment() {
    m_current++;
    render();
}

void ProgressBar::finish() {
    m_current = m_total;
    render();
    std::cout << std::endl;
}

void ProgressBar::set_prefix(const std::string& prefix) {
    m_prefix = prefix;
}

void ProgressBar::set_suffix(const std::string& suffix) {
    m_suffix = suffix;
}

void ProgressBar::render() {
    Terminal::clear_line();
    
    double percentage = (m_total > 0) ? 
        (static_cast<double>(m_current) / m_total) * 100.0 : 0.0;
    int filled = static_cast<int>((percentage / 100.0) * m_width);
    
    if (!m_prefix.empty()) {
        std::cout << m_prefix << " ";
    }
    
    std::cout << "[";
    
    for (int i = 0; i < m_width; i++) {
        if (i < filled) {
            Terminal::print_styled("=", Color::Cyan);
        } else if (i == filled) {
            Terminal::print_styled(">", Color::Cyan, Style::Bold);
        } else {
            std::cout << " ";
        }
    }
    
    std::cout << "] ";
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << percentage << "%";
    std::cout << oss.str();
    
    if (!m_suffix.empty()) {
        std::cout << " " << m_suffix;
    }
    
    std::cout << std::flush;
}

// MultiProgress implementation

MultiProgress::MultiProgress() = default;

MultiProgress::~MultiProgress() {
    stop();
}

int MultiProgress::add_task(const std::string& name, int total) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Task task;
    task.name = name;
    task.current = 0;
    task.total = total;
    task.finished = false;
    task.failed = false;
    
    m_tasks.push_back(task);
    return static_cast<int>(m_tasks.size()) - 1;
}

void MultiProgress::update_task(int task_id, int current, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (task_id >= 0 && task_id < static_cast<int>(m_tasks.size())) {
        m_tasks[task_id].current = current;
        m_tasks[task_id].message = message;
    }
}

void MultiProgress::finish_task(int task_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (task_id >= 0 && task_id < static_cast<int>(m_tasks.size())) {
        m_tasks[task_id].finished = true;
        m_tasks[task_id].current = m_tasks[task_id].total;
    }
}

void MultiProgress::fail_task(int task_id, const std::string& error) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (task_id >= 0 && task_id < static_cast<int>(m_tasks.size())) {
        m_tasks[task_id].failed = true;
        m_tasks[task_id].message = error;
    }
}

void MultiProgress::start() {
    m_running = true;
    m_render_thread = std::thread(&MultiProgress::render_loop, this);
}

void MultiProgress::stop() {
    m_running = false;
    
    if (m_render_thread.joinable()) {
        m_render_thread.join();
    }
}

void MultiProgress::render_loop() {
    Terminal::hide_cursor();
    
    while (m_running) {
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    Terminal::show_cursor();
}

void MultiProgress::render() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Move cursor up to overwrite previous render
    if (!m_tasks.empty()) {
        Terminal::move_up(static_cast<int>(m_tasks.size()));
    }
    
    for (const auto& task : m_tasks) {
        Terminal::clear_line();
        
        std::cout << "  ";
        
        if (task.failed) {
            Terminal::print_styled("✗", Color::Red, Style::Bold);
        } else if (task.finished) {
            Terminal::print_styled("✓", Color::Green, Style::Bold);
        } else {
            Terminal::print_styled("○", Color::Gray);
        }
        
        std::cout << " " << task.name;
        
        if (task.total > 0 && !task.finished && !task.failed) {
            double percentage = (static_cast<double>(task.current) / task.total) * 100.0;
            std::cout << " ";
            
            int bar_width = 15;
            int filled = static_cast<int>((percentage / 100.0) * bar_width);
            
            Terminal::print_styled("[", Color::Gray);
            for (int i = 0; i < bar_width; i++) {
                if (i < filled) {
                    Terminal::print_styled("█", Color::Cyan);
                } else {
                    Terminal::print_styled("░", Color::Gray);
                }
            }
            Terminal::print_styled("]", Color::Gray);
            
            std::ostringstream oss;
            oss << " " << std::fixed << std::setprecision(0) << percentage << "%";
            Terminal::print_styled(oss.str(), Color::Gray);
        }
        
        if (!task.message.empty()) {
            std::cout << " ";
            Terminal::print_styled(task.message, Color::Gray);
        }
        
        std::cout << std::endl;
    }
    
    std::cout << std::flush;
}

} // namespace iris::ui