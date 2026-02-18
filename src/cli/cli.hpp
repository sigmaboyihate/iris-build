#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace iris::cli {

    struct Option {
        std::string short_name;
        std::string long_name;
        std::string description;
        bool requires_value;
        std::string default_value;
    };

    struct Command {
        std::string name;
        std::string description;
        std::vector<Option> options;
        std::vector<std::string> positional_args;
        std::function<int(const std::map<std::string, std::string>&,
                          const std::vector<std::string>&)> handler;
    };

    class CLI {
    public:
        CLI(const std::string& name, const std::string& description);
        ~CLI() = default;

        void add_command(const Command& cmd);
        void add_global_option(const Option& opt);

        int run(int argc, char* argv[]);

        void print_help() const;
        void print_version() const;
        void print_command_help(const std::string& cmd_name) const;

    private:
        std::string m_name;
        std::string m_description;
        std::string m_version = "0.1.0";

        std::vector<Command> m_commands;
        std::vector<Option> m_global_options;

        void register_default_commands();
        std::pair<std::map<std::string, std::string>, std::vector<std::string>>
        parse_args(int argc, char* argv[], const Command* cmd);
    };

} // namespace iris::cli
