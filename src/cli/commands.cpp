#include "commands.hpp"
#include <iomanip>
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
    using namespace iris::ui;

    std::string source_dir = positional.empty() ? "." : positional[0];
    std::string build_dir = options.at("builddir");
    std::string build_type = options.at("buildtype");

    Terminal::header("Configuring Project");

    // check for iris.build file
    std::string build_file = source_dir + "/iris.build";
    if (!fs::exists(build_file)) {
        Terminal::error("No iris.build found in " + source_dir);
        Terminal::hint("Run 'iris init' to create a new project");
        return 1;
    }

    Terminal::info("Source directory", source_dir);
    Terminal::info("Build directory", build_dir);
    Terminal::info("Build type", build_type);

    // parse and interpret the build file
    try {
        lang::Parser parser;
        auto ast = parser.parse_file(build_file);

        lang::Interpreter interpreter;
        interpreter.set_variable("builddir", build_dir);
        interpreter.set_variable("buildtype", build_type);
        interpreter.set_variable("prefix", options.at("prefix"));
        
        auto config = interpreter.execute(ast);

        // create build directory
        fs::create_directories(build_dir);

        // generate build files
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
    using namespace iris::ui;

    std::string build_dir = options.count("builddir") && !options.at("builddir").empty() ? options.at("builddir") : "build";
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
        // clean implementation
    }

    try {
        core::Engine engine;
        engine.load_from_build_dir(build_dir);

        auto build_start = std::chrono::steady_clock::now();
        
        int result = engine.build(
            target,
            jobs.empty() ? 0 : std::stoi(jobs),
            verbose,
            nullptr
        );

        auto build_end = std::chrono::steady_clock::now();
        double secs = std::chrono::duration<double>(build_end - build_start).count();

        if (result == 0) {
            Terminal::print_styled("  ✓ ", Color::Green, Style::Bold);
            std::cout << "Build complete";
            Terminal::print_styled(" [", Color::Gray);
            std::cout << std::fixed << std::setprecision(2) << secs << "s";
            Terminal::print_styled("]\n", Color::Gray);
        } else {
            Terminal::print_styled("  ✗ ", Color::Red, Style::Bold);
            std::cout << "Build failed\n";
            return result;
        }

    } catch (const std::exception& e) {
        Terminal::error("Build error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

    using namespace iris::ui;
int cmd_clean(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional) {
    using namespace iris::ui;
    (void)positional;

    std::string build_dir = options.count("builddir") && !options.at("builddir").empty() ? options.at("builddir") : "build";
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
            // clean only build artifacts, keep config
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
    using namespace iris::ui;

    std::string name = options.count("name") && !options.at("name").empty() 
                       ? options.at("name") 
                       : fs::current_path().filename().string();
    std::string lang = options.at("lang");
    bool is_lib = options.count("lib") && options.at("lib") == "true";

    Terminal::header("Initializing New Project");

    // check if iris.build already exists
    if (fs::exists("iris.build")) {
        Terminal::error("iris.build already exists in this directory");
        return 1;
    }

    Terminal::info("Project name", name);
    Terminal::info("Language", lang);
    Terminal::info("Type", is_lib ? "library" : "executable");

    // create project structure
    fs::create_directories("src");
    fs::create_directories("include");

    // generate iris.build
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

    // create sample source file
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

    // create .gitignore
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
    using namespace iris::ui;

    // first build
    int build_result = cmd_build({{"verbose", "false"}}, {});
    if (build_result != 0) {
        return build_result;
    }

    Terminal::header("Running");

    std::string target = options.count("target") ? options.at("target") : "";
    std::string args = options.count("args") ? options.at("args") : "";

    // find executable
    std::string exe_path;
    if (!target.empty()) {
        exe_path = "build/" + target;
    } else {
        // ind first executable in build directory
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
    using namespace iris::ui;

    Terminal::header("Running Tests");

    bool verbose = options.count("verbose") && options.at("verbose") == "true";
    (void)verbose;
    std::string filter = options.count("filter") ? options.at("filter") : "";
    int timeout = std::stoi(options.at("timeout"));
    (void)timeout;

    // build first
    int build_result = cmd_build({}, {});
    if (build_result != 0) {
        return build_result;
    }

    // find and run tests
    int passed = 0, failed = 0, skipped = 0;

    // look for test executables
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
    using namespace iris::ui;

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
    using namespace iris::ui;

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


int cmd_install(const std::map<std::string, std::string>& options,
                const std::vector<std::string>& positional) {
    using namespace iris::ui;
    (void)positional;

    std::string build_dir = options.count("builddir") && !options.at("builddir").empty()
                            ? options.at("builddir") : "build";
    std::string prefix = options.at("prefix");
    std::string destdir = options.count("destdir") ? options.at("destdir") : "";
    bool dry_run = options.count("dry-run") && options.at("dry-run") == "true";
    bool do_strip = options.count("strip") && options.at("strip") == "true";

    Terminal::header("Installing");

    std::string config_file = build_dir + "/iris-config.json";
    if (!fs::exists(config_file)) {
        Terminal::error("No configuration found in " + build_dir);
        Terminal::hint("Run 'iris setup' and 'iris build' first");
        return 1;
    }

    std::string install_prefix = destdir.empty() ? prefix : destdir + prefix;
    std::string bin_dir = install_prefix + "/bin";
    std::string lib_dir = install_prefix + "/lib";

    Terminal::info("Prefix", prefix);
    if (!destdir.empty()) {
        Terminal::info("DESTDIR", destdir);
    }
    Terminal::info("Binary dir", bin_dir);
    Terminal::info("Library dir", lib_dir);
    std::cout << "\n";

    int installed_count = 0;
    int failed_count = 0;

    for (const auto& entry : fs::directory_iterator(build_dir)) {
        if (!entry.is_regular_file()) continue;
        
        fs::path path = entry.path();
        std::string filename = path.filename().string();
        
        if (filename == "build.ninja" || 
            filename == "Makefile" ||
            filename == "iris-config.json" ||
            filename.find(".ninja") != std::string::npos ||
            filename.find(".o") != std::string::npos ||
            filename.find(".d") != std::string::npos) {
            continue;
        }

        auto perms = fs::status(path).permissions();
        bool is_executable = (perms & fs::perms::owner_exec) != fs::perms::none;
        bool is_shared_lib = filename.find(".so") != std::string::npos ||
                             filename.find(".dylib") != std::string::npos;
        bool is_static_lib = filename.length() > 2 && 
                             filename.substr(filename.length() - 2) == ".a";

        std::string dest_dir;
        std::string dest_path;
        
        if (is_shared_lib || is_static_lib) {
            dest_dir = lib_dir;
            dest_path = lib_dir + "/" + filename;
        } else if (is_executable) {
            dest_dir = bin_dir;
            dest_path = bin_dir + "/" + filename;
        } else {
            continue;
        }

        if (dry_run) {
            std::cout << "  ";
            Terminal::print_styled("WOULD INSTALL", Color::Cyan);
            std::cout << "  " << path.string() << " -> " << dest_path << "\n";
            installed_count++;
            continue;
        }

        try {
            fs::create_directories(dest_dir);
        } catch (const fs::filesystem_error& e) {
            Terminal::error("Cannot create directory " + dest_dir + ": " + e.what());
            Terminal::hint("Try running with sudo for system-wide installation");
            failed_count++;
            continue;
        }

        if (do_strip && is_executable && !is_static_lib) {
            std::string strip_cmd = "strip " + path.string() + " 2>/dev/null";
            std::system(strip_cmd.c_str());
        }

        try {
            fs::copy_file(path, dest_path, fs::copy_options::overwrite_existing);
            
            if (is_executable) {
                fs::permissions(dest_path, 
                    fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec |
                    fs::perms::group_read | fs::perms::group_exec |
                    fs::perms::others_read | fs::perms::others_exec);
            }
            
            std::cout << "  ";
            Terminal::print_styled("INSTALL", Color::Green);
            std::cout << "  " << filename;
            Terminal::print_styled(" -> ", Color::Gray);
            std::cout << dest_path << "\n";
            installed_count++;
            
        } catch (const fs::filesystem_error& e) {
            std::cout << "  ";
            Terminal::print_styled("FAILED", Color::Red);
            std::cout << "   " << filename << ": " << e.what() << "\n";
            failed_count++;
        }
    }

    std::cout << "\n";
    
    if (dry_run) {
        Terminal::info("Dry run", "No files were modified");
        Terminal::hint("Run without --dry-run to perform actual installation");
    } else if (failed_count > 0) {
        Terminal::warning("Installed " + std::to_string(installed_count) + 
                         " files, " + std::to_string(failed_count) + " failed");
        if (prefix.find("/usr") == 0) {
            Terminal::hint("For system directories, try: sudo iris install");
        }
        return 1;
    } else if (installed_count > 0) {
        Terminal::success("Installed " + std::to_string(installed_count) + " files to " + prefix);
    } else {
        Terminal::warning("No files to install");
        Terminal::hint("Make sure you have built the project with 'iris build'");
    }

    return 0;
}

} // namespace iris::cli::commands
