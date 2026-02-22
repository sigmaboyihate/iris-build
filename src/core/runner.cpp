#include "runner.hpp"

#include <array>
#include <chrono>
#include <thread>
#include <future>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace iris::core {

Runner::Runner() = default;

void Runner::set_working_dir(const std::string& dir) {
    m_working_dir = dir;
}

void Runner::set_env(const std::string& key, const std::string& value) {
    m_env[key] = value;
}

void Runner::clear_env() {
    m_env.clear();
}

RunResult Runner::run(const std::string& command) {
    return execute(command);
}

RunResult Runner::run(const std::vector<std::string>& args) {
    std::string command;
    for (const auto& arg : args) {
        if (!command.empty()) command += " ";
        // quote args with spaces
        if (arg.find(' ') != std::string::npos) {
            command += "\"" + arg + "\"";
        } else {
            command += arg;
        }
    }
    return execute(command);
}

RunResult Runner::execute(const std::string& command) {
    RunResult result;
    result.exit_code = -1;

    auto start = std::chrono::high_resolution_clock::now();

    m_running = true;

    std::string full_command = command;
    if (!m_working_dir.empty()) {
        full_command = "cd \"" + m_working_dir + "\" && " + command;
    }

    // set environment variables
    for (const auto& [key, value] : m_env) {
#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }

#ifdef _WIN32
    // windows implementation
    FILE* pipe = _popen(full_command.c_str(), "r");
#else
    // unix implementation
    FILE* pipe = popen((full_command + " 2>&1").c_str(), "r");
#endif

    if (pipe) {
        std::array<char, 256> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            if (m_cancelled) break;
            result.stdout_output += buffer.data();
        }

#ifdef _WIN32
        result.exit_code = _pclose(pipe);
#else
        int status = pclose(pipe);
        result.exit_code = WEXITSTATUS(status);
#endif
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.elapsed_seconds = std::chrono::duration<double>(end - start).count();

    m_running = false;
    m_cancelled = false;

    return result;
}

void Runner::run_async(const std::string& command,
                       OutputCallback on_stdout,
                       OutputCallback on_stderr,
                       std::function<void(int)> on_complete) {
    std::thread([this, command, on_stdout, on_complete]() {
        auto result = execute(command);
        if (on_stdout) {
            on_stdout(result.stdout_output);
        }
        if (on_complete) {
            on_complete(result.exit_code);
        }
    }).detach();
}

std::vector<RunResult> Runner::run_parallel(const std::vector<std::string>& commands,
                                             int max_parallel) {
    if (max_parallel <= 0) {
        max_parallel = std::thread::hardware_concurrency();
        if (max_parallel == 0) max_parallel = 4;
    }

    std::vector<std::future<RunResult>> futures;
    std::vector<RunResult> results;

    for (size_t i = 0; i < commands.size(); i++) {
        if (futures.size() >= static_cast<size_t>(max_parallel)) {
            // wait for one to complete
            for (auto& f : futures) {
                if (f.valid()) {
                    results.push_back(f.get());
                    break;
                }
            }
        }

        futures.push_back(std::async(std::launch::async, [this, cmd = commands[i]]() {
            return execute(cmd);
        }));
    }

    // collect remaining results
    for (auto& f : futures) {
        if (f.valid()) {
            results.push_back(f.get());
        }
    }

    return results;
}

void Runner::cancel() {
    m_cancelled = true;
}

bool Runner::is_running() const {
    return m_running;
}

} // namespace iris::core
