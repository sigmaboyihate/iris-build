#include "commands.hpp"
#include "../core/engine.hpp"
#include "../lang/parser.hpp"
#include "../lang/interpreter.hpp"
#include "../ui/terminal.hpp"
#include "../core/graph.hpp"
#include "../ui/progress.hpp"
#include "../util/fs.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace iris::cli::commands {

int cmd_setup(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional) {
    using namespace ui;

    std::string source_dir = positional.empty() ? "." : positional[0];
    std::string build_dir = options.at("builddir");
    std::string build_type = options.at("buildtype");

    Terminal::header("Configuring Project");

    // Check for iris.build file
    std::string build_file = source_dir + "/iris.build";
    if (!fs::exists(build_file)) {
        Terminal::error("No iris.build found in " + source_dir);
        Terminal::hint("Run 'iris init' to create a new project");
        return 1;
    }

    Terminal::info("Source directory", source_dir);
    Terminal::info("Build directory", build_dir);
    Terminal::info("Build type", build_type);

    // Parse and interpret the build file
    try {
        lang::Parser parser;
        auto ast = parser.parse_file(build_file);

        lang::Interpreter interpreter;
        interpreter.set_variable("builddir", build_dir);
        interpreter.set_variable("buildtype", build_type);
        interpreter.set_variable("prefix", options.at("prefix"));
        
        auto config = interpreter.execute(ast);

        // Create build directory
        fs::create_directories(build_dir);

        // Generate build files
        core::Engine engine(config);
        engine.generate_build_files(build_dir, options.at("backend"));

        std::cout << "\n";
        Terminal::success("Configuration complete!");
        Terminal::hint("Run 'iris build' to compile the project");

    } catch (const std::exception& e) {
        Terminal::error("Configuration failed: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

int cmd_build(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional) {
    using namespace ui;

    std::string build_dir = "build";
    if (!fs::exists(build_dir)) {
        Terminal::error("Build directory not found");
        Terminal::hint("Run 'iris setup .' first to configure the project");
        return 1;
    }

    Terminal::header("Building Project");

    auto start_time = std::chrono::high_resolution_clock::now();

    bool verbose = options.count("verbose") && options.at("verbose") == "true";
    (void)verbose;
    bool clean_first = options.count("clean") && options.at("clean") == "true";
    std::string jobs = options.count("jobs") ? options.at("jobs") : "";
    std::string target = options.count("target") ? options.at("target") : "";

    if (clean_first) {
        Terminal::info("Cleaning build directory...");
        // Clean implementation
    }

    try {
        core::Engine engine;
        engine.load_from_build_dir(build_dir);

        Progress progress;
        progress.start("Compiling");

        int result = engine.build(
            target,
            jobs.empty() ? 0 : std::stoi(jobs),
            verbose,
            [&progress](const std::string& task, int current, int total) {
                progress.update(task, current, total);
            }
        );

        progress.finish();

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();

        if (result == 0) {
            std::cout << "\n";
            Terminal::success("Build completed in " + 
                std::to_string(duration / 1000.0) + "s");
        } else {
            std::cout << "\n";
            Terminal::error("Build failed");
            return result;
        }

    } catch (const std::exception& e) {
        Terminal::error("Build error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

int cmd_clean(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional) {
    using namespace ui;

    std::string build_dir = "build";
    bool clean_all = options.count("all") && options.at("all") == "true";

    Terminal::header("Cleaning");

    if (clean_all) {
        if (fs::exists(build_dir)) {
            Terminal::info("Removing", build_dir);
            fs::remove_all(build_dir);
        }
        if (fs::exists(".iris-cache")) {
            Terminal::info("Removing", ".iris-cache");
            fs::remove_all(".iris-cache");
        }
    } else {
        if (fs::exists(build_dir)) {
            // Clean only build artifacts, keep config
            for (const auto& entry : fs::directory_iterator(build_dir)) {
                if (entry.path().filename() != "iris-config.json") {
                    Terminal::info("Removing", entry.path().string());
                    fs::remove_all(entry.path());
                }
            }
        }
    }

    Terminal::success("Clean complete");
    return 0;
}

int cmd_init(const std::map<std::string, std::string>& options,
             const std::vector<std::string>& positional) {
    using namespace ui;

    std::string name = options.count("name") && !options.at("name").empty() 
                       ? options.at("name") 
                       : fs::current_path().filename().string();
    std::string lang = options.at("lang");
    bool is_lib = options.count("lib") && options.at("lib") == "true";

    Terminal::header("Initializing New Project");

    // Check if iris.build already exists
    if (fs::exists("iris.build")) {
        Terminal::error("iris.build already exists in this directory");
        return 1;
    }

    Terminal::info("Project name", name);
    Terminal::info("Language", lang);
    Terminal::info("Type", is_lib ? "library" : "executable");

    // Create project structure
    fs::create_directories("src");
    fs::create_directories("include");

    // Generate iris.build
    std::ofstream build_file("iris.build");
    build_file << R"(# Iris Build Configuration
# Generated by iris init

project ")" << name << R"(" do
    version = "0.1.0"
    license = "MIT"
    
    # Supported: :c, :cpp, :mixed
    lang = :)" << lang << R"(
    
    # C/C++ standard
    std = ")" << (lang == "c" ? "c17" : "c++20") << R"("
end

# Compiler configuration
compiler do
    # Compiler flags for different build types
    if buildtype == "debug" do
        flags = ["-g", "-O0", "-DDEBUG"]
    end
    
    if buildtype == "release" do
        flags = ["-O3", "-DNDEBUG", "-march=native"]
    end
    
    # Warning flags
    warnings = ["-Wall", "-Wextra", "-Wpedantic"]
end

)";

    if (is_lib) {
        build_file << R"(# Library target
library ")" << name << R"(" do
    sources = glob("src/**/*.)" << (lang == "c" ? "c" : "cpp") << R"(")
    includes = ["include/"]
    
    # Public headers for installation
    public_headers = glob("include/**/*.h")
end
)";
    } else {
        build_file << R"(# Executable target
executable ")" << name << R"(" do
    sources = glob("src/**/*.)" << (lang == "c" ? "c" : "cpp") << R"(")
    includes = ["include/"]
    
    # External dependencies
    # deps = [find_package("pthread")]
end
)";
    }

    build_file.close();

    // Create sample source file
    std::string src_ext = (lang == "c") ? "c" : "cpp";
    std::ofstream src_file("src/main." + src_ext);
    
    if (lang == "c") {
        src_file << R"(#include <stdio.h>

int main(void) {
    printf("Hello from )" << name << R"(!\n");
    return 0;
}
)";
    } else {
        src_file << R"(#include <iostream>

int main() {
    std::cout << "Hello from )" << name << R"(!" << std::endl;
    return 0;
}
)";
    }
    src_file.close();

    // Create .gitignore
    std::ofstream gitignore(".gitignore");
    gitignore << R"(# Build directories
build/
.iris-cache/

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# Compiled
*.o
*.a
*.so
*.dylib
)";
    gitignore.close();

    std::cout << "\n";
    Terminal::success("Project initialized!");
    std::cout << "\n";
    Terminal::print_styled("  Created files:\n", Color::Gray);
    std::cout << "    • iris.build\n";
    std::cout << "    • src/main." << src_ext << "\n";
    std::cout << "    • .gitignore\n";
    std::cout << "\n";
    Terminal::hint("Run 'iris setup . && iris build' to compile");

    return 0;
}

int cmd_run(const std::map<std::string, std::string>& options,
            const std::vector<std::string>& positional) {
    using namespace ui;

    // First build
    int build_result = cmd_build({{"verbose", "false"}}, {});
    if (build_result != 0) {
        return build_result;
    }

    Terminal::header("Running");

    std::string target = options.count("target") ? options.at("target") : "";
    std::string args = options.count("args") ? options.at("args") : "";

    // Find executable
    std::string exe_path;
    if (!target.empty()) {
        exe_path = "build/" + target;
    } else {
        // Find first executable in build directory
        for (const auto& entry : fs::directory_iterator("build")) {
            if (fs::is_regular_file(entry) && 
                (entry.path().extension().empty() || 
                 entry.path().extension() == ".exe")) {
                auto perms = fs::status(entry).permissions();
                if ((perms & fs::perms::owner_exec) != fs::perms::none) {
                    exe_path = entry.path().string();
                    break;
                }
            }
        }
    }

    if (exe_path.empty() || !fs::exists(exe_path)) {
        Terminal::error("No executable found");
        Terminal::hint("Specify a target with --target=<name>");
        return 1;
    }

    Terminal::info("Executing", exe_path);
    std::cout << "\n";
    Terminal::separator();

    std::string cmd = exe_path;
    if (!args.empty()) {
        cmd += " " + args;
    }

    int result = std::system(cmd.c_str());

    Terminal::separator();
    std::cout << "\n";

    if (result == 0) {
        Terminal::success("Process exited with code 0");
    } else {
        Terminal::warning("Process exited with code " + std::to_string(result));
    }

    return result;
}

int cmd_test(const std::map<std::string, std::string>& options,
             const std::vector<std::string>& positional) {
    using namespace ui;

    Terminal::header("Running Tests");

    bool verbose = options.count("verbose") && options.at("verbose") == "true";
    (void)verbose;
    std::string filter = options.count("filter") ? options.at("filter") : "";
    int timeout = std::stoi(options.at("timeout"));
    (void)timeout;

    // Build first
    int build_result = cmd_build({}, {});
    if (build_result != 0) {
        return build_result;
    }

    // Find and run tests
    int passed = 0, failed = 0, skipped = 0;

    // Look for test executables
    std::string test_dir = "build/tests";
    if (!fs::exists(test_dir)) {
        Terminal::warning("No tests found");
        Terminal::hint("Create a 'test' target in your iris.build file");
        return 0;
    }

    for (const auto& entry : fs::directory_iterator(test_dir)) {
        if (!fs::is_regular_file(entry)) continue;

        std::string test_name = entry.path().filename().string();
        
        if (!filter.empty() && test_name.find(filter) == std::string::npos) {
            skipped++;
            continue;
        }

        std::cout << "  ";
        Terminal::print_styled("TEST", Color::Blue, Style::Bold);
        std::cout << "  " << test_name << " ... ";

        int result = std::system(entry.path().string().c_str());

        if (result == 0) {
            Terminal::print_styled("PASS\n", Color::Green, Style::Bold);
            passed++;
        } else {
            Terminal::print_styled("FAIL\n", Color::Red, Style::Bold);
            failed++;
        }
    }

    std::cout << "\n";
    Terminal::separator();
    std::cout << "  Results: ";
    Terminal::print_styled(std::to_string(passed) + " passed", Color::Green);
    std::cout << ", ";
    if (failed > 0) {
        Terminal::print_styled(std::to_string(failed) + " failed", Color::Red);
    } else {
        std::cout << "0 failed";
    }
    std::cout << ", " << skipped << " skipped\n";

    return failed > 0 ? 1 : 0;
}

int cmd_info(const std::map<std::string, std::string>& options,
             const std::vector<std::string>& positional) {
    using namespace ui;

    Terminal::header("Project Information");

    if (!fs::exists("iris.build")) {
        Terminal::error("No iris.build found in current directory");
        return 1;
    }

    try {
        lang::Parser parser;
        auto ast = parser.parse_file("iris.build");

        lang::Interpreter interpreter;
        auto config = interpreter.execute(ast);

        Terminal::info("Name", config.project_name);
        Terminal::info("Version", config.version);
        Terminal::info("Language", config.language);

        if (options.count("targets") && options.at("targets") == "true") {
            std::cout << "\n";
            Terminal::print_styled("Targets:\n", Color::Yellow, Style::Bold);
            for (const auto& target : config.targets) {
                std::cout << "  • " << target.name;
                if (target.type == core::TargetType::Executable) {
                    Terminal::print_styled(" (executable)", Color::Gray);
                } else if (target.type == core::TargetType::Library) {
                    Terminal::print_styled(" (library)", Color::Gray);
                }
                std::cout << "\n";
            }
        }

        if (options.count("deps") && options.at("deps") == "true") {
            std::cout << "\n";
            Terminal::print_styled("Dependencies:\n", Color::Yellow, Style::Bold);
            for (const auto& dep : config.dependencies) {
                std::cout << "  • " << dep.name;
                if (!dep.version.empty()) {
                    std::cout << " " << dep.version;
                }
                std::cout << "\n";
            }
        }

    } catch (const std::exception& e) {
        Terminal::error("Failed to parse iris.build: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

int cmd_graph(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional) {
    using namespace ui;

    Terminal::header("Generating Dependency Graph");

    std::string output = options.at("output");
    std::string format = options.at("format");

    if (!fs::exists("iris.build")) {
        Terminal::error("No iris.build found in current directory");
        return 1;
    }

    try {
        lang::Parser parser;
        auto ast = parser.parse_file("iris.build");

        lang::Interpreter interpreter;
        auto config = interpreter.execute(ast);

        core::Graph graph(config);
        
        std::ofstream out(output);
        if (format == "dot") {
            out << graph.to_dot();
        } else if (format == "json") {
            out << graph.to_json();
        }
        out.close();

        Terminal::success("Graph written to " + output);

    } catch (const std::exception& e) {
        Terminal::error("Failed to generate graph: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

} // namespace iris::cli::commands