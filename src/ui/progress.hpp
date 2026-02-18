#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

namespace iris::ui {

class Progress {
public:
    Progress();
    ~Progress();

    void start(const std::string& task_name);
    void update(const std::string& task_name, int current, int total);
    void update(double percentage);
    void set_message(const std::string& message);
    void finish();
    void fail(const std::string& error = "");

    void increment();
    void set_total(int total);

    bool is_running() const { return m_running; }

private:
    std::string m_task_name;
    std::string m_current_message;
    int m_current = 0;
    int m_total = 0;
    
    std::atomic<bool> m_running{false};
    std::thread m_spinner_thread;
    std::mutex m_mutex;
    
    std::chrono::steady_clock::time_point m_start_time;
    
    void render();
    void spinner_loop();
    std::string format_duration(double seconds) const;
    std::string get_spinner_frame() const;
    
    static const std::vector<std::string> SPINNER_FRAMES;
    static const std::vector<std::string> PROGRESS_CHARS;
    int m_spinner_index = 0;
};

class ProgressBar {
public:
    ProgressBar(int total, int width = 40);
    
    void update(int current);
    void increment();
    void finish();
    
    void set_prefix(const std::string& prefix);
    void set_suffix(const std::string& suffix);

private:
    int m_total;
    int m_current = 0;
    int m_width;
    std::string m_prefix;
    std::string m_suffix;
    
    void render();
};

class MultiProgress {
public:
    struct Task {
        std::string name;
        int current;
        int total;
        bool finished;
        bool failed;
        std::string message;
    };

    MultiProgress();
    ~MultiProgress();

    int add_task(const std::string& name, int total);
    void update_task(int task_id, int current, const std::string& message = "");
    void finish_task(int task_id);
    void fail_task(int task_id, const std::string& error = "");

    void start();
    void stop();

private:
    std::vector<Task> m_tasks;
    std::mutex m_mutex;
    std::atomic<bool> m_running{false};
    std::thread m_render_thread;

    void render();
    void render_loop();
};

} // namespace iris::ui