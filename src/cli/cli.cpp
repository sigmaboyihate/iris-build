#include "cli.hpp"
#include "commands.hpp"
#include "../ui/terminal.hpp"

#include <iostream>
#include <iomanip>
#include <algorithm>

namespace iris::cli {

CLI::CLI(const std::string& name, const std::string& description)
    : m_name(name), m_description(description) {
    register_default_commands();
}

void CLI::register_default_commands() {
    // Setup command
    add_command({
        "setup",
        "Configure a build directory",
        {
            {"-b", "--builddir", "Build directory path", true, "build"},
            {"-p", "--prefix", "Installation prefix", true, "/usr/local"},
            {"", "--buildtype", "Build type (debug/release/minsize)", true, "debug"},
            {"", "--backend", "Build backend (ninja/make)", true, "ninja"}
        },
        {"source_dir"},
        commands::cmd_setup
    });

    // Build command
    add_command({
        "build",
        "Build the project",
        {
            {"-j", "--jobs", "Number of parallel jobs", true, ""},
            {"-v", "--verbose", "Verbose output", false, ""},
            {"-c", "--clean", "Clean before building", false, ""},
            {"", "--target", "Specific target to build", true, ""}
        },
        {},
        commands::cmd_build
    });

    // Clean command
    add_command({
        "clean",
        "Clean build artifacts",
        {
            {"-a", "--all", "Remove entire build directory", false, ""}
        },
        {},
        commands::cmd_clean
    });

    // Init command
    add_command({
        "init",
        "Initialize a new Iris project",
        {
            {"-n", "--name", "Project name", true, ""},
            {"-l", "--lang", "Project language (c/cpp/mixed)", true, "cpp"},
            {"", "--lib", "Create a library project", false, ""},
            {"", "--exe", "Create an executable project", false, ""}
        },
        {},
        commands::cmd_init
    });

    // Run command
    add_command({
        "run",
        "Build and run an executable target",
        {
            {"", "--target", "Target to run", true, ""},
            {"", "--args", "Arguments to pass", true, ""}
        },
        {},
        commands::cmd_run
    });

    // Test command
    add_command({
        "test",
        "Run project tests",
        {
            {"-v", "--verbose", "Verbose test output", false, ""},
            {"", "--filter", "Test name filter", true, ""},
            {"", "--timeout", "Test timeout in seconds", true, "60"}
        },
        {},
        commands::cmd_test
    });

    // Info command
    add_command({
        "info",
        "Show project information",
        {
            {"", "--targets", "List all targets", false, ""},
            {"", "--options", "List all options", false, ""},
            {"", "--deps", "Show dependency tree", false, ""}
        },
        {},
        commands::cmd_info
    });

    // Graph command
    add_command({
        "graph",
        "Generate dependency graph",
        {
            {"-o", "--output", "Output file", true, "graph.dot"},
            {"", "--format", "Output format (dot/json)", true, "dot"}
        },
        {},
        commands::cmd_graph
    });

    // Global options
    add_global_option({"-h", "--help", "Show help message", false, ""});
    add_global_option({"-V", "--version", "Show version", false, ""});
    add_global_option({"", "--color", "Color output (auto/always/never)", true, "auto"});
    add_global_option({"", "--quiet", "Suppress non-error output", false, ""});
}

void CLI::add_command(const Command& cmd) {
    m_commands.push_back(cmd);
}

void CLI::add_global_option(const Option& opt) {
    m_global_options.push_back(opt);
}

int CLI::run(int argc, char* argv[]) {
    using namespace ui;
    
    if (argc < 2) {
        print_help();
        return 0;
    }

    std::string first_arg = argv[1];

    // Check global options first
    if (first_arg == "-h" || first_arg == "--help") {
        print_help();
        return 0;
    }
    if (first_arg == "-V" || first_arg == "--version") {
        print_version();
        return 0;
    }

    // Find command
    auto cmd_it = std::find_if(m_commands.begin(), m_commands.end(),
        [&first_arg](const Command& c) { return c.name == first_arg; });

    if (cmd_it == m_commands.end()) {
        Terminal::error("Unknown command: " + first_arg);
        Terminal::hint("Run 'iris --help' for usage information");
        return 1;
    }

    // Check for command-specific help
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_command_help(cmd_it->name);
            return 0;
        }
    }

    // Parse arguments and run command
    try {
        auto [options, positional] = parse_args(argc, argv, &(*cmd_it));
        return cmd_it->handler(options, positional);
    } catch (const std::exception& e) {
        Terminal::error(e.what());
        return 1;
    }
}

std::pair<std::map<std::string, std::string>, std::vector<std::string>>
CLI::parse_args(int argc, char* argv[], const Command* cmd) {
    std::map<std::string, std::string> options;
    std::vector<std::string> positional;

    // Set defaults
    for (const auto& opt : cmd->options) {
        if (!opt.default_value.empty()) {
            std::string key = opt.long_name.empty() ? opt.short_name : opt.long_name;
            if (key.substr(0, 2) == "--") key = key.substr(2);
            else if (key[0] == '-') key = key.substr(1);
            options[key] = opt.default_value;
        }
    }

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg[0] == '-') {
            // Find matching option
            const Option* matched = nullptr;
            for (const auto& opt : cmd->options) {
                if (arg == opt.short_name || arg == opt.long_name) {
                    matched = &opt;
                    break;
                }
                // Handle --option=value
                if (arg.find('=') != std::string::npos) {
                    std::string opt_name = arg.substr(0, arg.find('='));
                    if (opt_name == opt.long_name) {
                        matched = &opt;
                        break;
                    }
                }
            }

            if (!matched) {
                throw std::runtime_error("Unknown option: " + arg);
            }

            std::string key = matched->long_name.empty() ? 
                              matched->short_name : matched->long_name;
            if (key.substr(0, 2) == "--") key = key.substr(2);
            else if (key[0] == '-') key = key.substr(1);

            if (matched->requires_value) {
                if (arg.find('=') != std::string::npos) {
                    options[key] = arg.substr(arg.find('=') + 1);
                } else if (i + 1 < argc) {
                    options[key] = argv[++i];
                } else {
                    throw std::runtime_error("Option " + arg + " requires a value");
                }
            } else {
                options[key] = "true";
            }
        } else {
            positional.push_back(arg);
        }
    }

    return {options, positional};
}

void CLI::print_help() const {
    using namespace ui;

    std::cout << "\n";
    Terminal::print_styled("  ╦╦═╗╦╔═╗  ", Color::Cyan, Style::Bold);
    Terminal::print_styled("Build System\n", Color::White, Style::Bold);
    Terminal::print_styled("  ║╠╦╝║╚═╗  ", Color::Cyan, Style::Bold);
    Terminal::print_styled("v" + m_version + "\n", Color::Gray);
    Terminal::print_styled("  ╩╩╚═╩╚═╝  ", Color::Cyan, Style::Bold);
    Terminal::print_styled(m_description + "\n\n", Color::Gray);

    Terminal::print_styled("USAGE:\n", Color::Yellow, Style::Bold);
    std::cout << "    iris <COMMAND> [OPTIONS]\n\n";

    Terminal::print_styled("COMMANDS:\n", Color::Yellow, Style::Bold);
    
    size_t max_len = 0;
    for (const auto& cmd : m_commands) {
        max_len = std::max(max_len, cmd.name.length());
    }

    for (const auto& cmd : m_commands) {
        std::cout << "    ";
        Terminal::print_styled(cmd.name, Color::Green);
        std::cout << std::string(max_len - cmd.name.length() + 4, ' ');
        std::cout << cmd.description << "\n";
    }

    std::cout << "\n";
    Terminal::print_styled("GLOBAL OPTIONS:\n", Color::Yellow, Style::Bold);
    
    for (const auto& opt : m_global_options) {
        std::cout << "    ";
        if (!opt.short_name.empty()) {
            Terminal::print_styled(opt.short_name, Color::Green);
            std::cout << ", ";
        } else {
            std::cout << "    ";
        }
        Terminal::print_styled(opt.long_name, Color::Green);
        std::cout << std::string(20 - opt.long_name.length(), ' ');
        std::cout << opt.description << "\n";
    }

    std::cout << "\n";
    Terminal::print_styled("EXAMPLES:\n", Color::Yellow, Style::Bold);
    std::cout << "    iris init --name myproject\n";
    std::cout << "    iris setup . --builddir=build\n";
    std::cout << "    iris build -j8\n";
    std::cout << "    iris run --target=myapp\n";
    std::cout << "\n";
    Terminal::hint("Run 'iris <command> --help' for more info on a command");
    std::cout << "\n";
}

void CLI::print_version() const {
    using namespace ui;
    Terminal::print_styled("iris ", Color::Cyan, Style::Bold);
    std::cout << m_version << "\n";
}

void CLI::print_command_help(const std::string& cmd_name) const {
    using namespace ui;

    auto it = std::find_if(m_commands.begin(), m_commands.end(),
        [&cmd_name](const Command& c) { return c.name == cmd_name; });
    
    if (it == m_commands.end()) return;

    const auto& cmd = *it;

    std::cout << "\n";
    Terminal::print_styled("iris " + cmd.name, Color::Cyan, Style::Bold);
    std::cout << " - " << cmd.description << "\n\n";

    Terminal::print_styled("USAGE:\n", Color::Yellow, Style::Bold);
    std::cout << "    iris " << cmd.name << " [OPTIONS]";
    for (const auto& pos : cmd.positional_args) {
        std::cout << " <" << pos << ">";
    }
    std::cout << "\n\n";

    if (!cmd.options.empty()) {
        Terminal::print_styled("OPTIONS:\n", Color::Yellow, Style::Bold);
        for (const auto& opt : cmd.options) {
            std::cout << "    ";
            if (!opt.short_name.empty()) {
                Terminal::print_styled(opt.short_name, Color::Green);
                std::cout << ", ";
            } else {
                std::cout << "    ";
            }
            Terminal::print_styled(opt.long_name, Color::Green);
            
            int padding = 20 - static_cast<int>(opt.long_name.length());
            if (padding < 1) padding = 1;
            std::cout << std::string(padding, ' ');
            // hi how are you doing there pal? snoooping in my code arent ya rascal?
            std::cout << opt.description;
            if (!opt.default_value.empty()) {
                std::cout << " [default: " << opt.default_value << "]";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace iris::cli
