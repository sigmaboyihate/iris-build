#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

namespace iris::core {

struct RunResult {
    int exit_code;
    std::string stdout_output;
    std::string stderr_output;
    double elapsed_seconds;
};

class Runner {
public:
    using OutputCallback = std::function<void(const std::string&)>;

    Runner();
    ~Runner() = default;

    void set_working_dir(const std::string& dir);
    void set_env(const std::string& key, const std::string& value);
    void clear_env();

    RunResult run(const std::string& command);
    RunResult run(const std::vector<std::string>& args);

    // async execution with callbacks
    void run_async(const std::string& command,
                   OutputCallback on_stdout = nullptr,
                   OutputCallback on_stderr = nullptr,
                   std::function<void(int)> on_complete = nullptr);

    // run multiple commands in parallel
    std::vector<RunResult> run_parallel(const std::vector<std::string>& commands,
                                         int max_parallel = 0);

    void cancel();
    bool is_running() const;

private:
    std::string m_working_dir;
    std::map<std::string, std::string> m_env;
    bool m_cancelled = false;
    bool m_running = false;

    RunResult execute(const std::string& command);
};

} // namespace iris::core
