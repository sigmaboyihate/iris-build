#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace iris::core {

    enum class TargetType {
        Executable,
        Library,
        StaticLibrary,
        SharedLibrary,
        Object,
        Custom
    };

    struct Target {
        std::string name;
        TargetType type = TargetType::Executable;
        std::vector<std::string> sources;
        std::vector<std::string> includes;
        std::vector<std::string> flags;
        std::vector<std::string> link_flags;
        std::vector<std::string> dependencies;
        std::map<std::string, std::string> defines;
    };

    struct Dependency {
        std::string name;
        std::string version;
        std::string type;  // system, pkg-config, cmake, subproject
        std::vector<std::string> include_dirs;
        std::vector<std::string> link_dirs;
        std::vector<std::string> libraries;
    };

    struct BuildConfig {
        std::string project_name;
        std::string version;
        std::string language;
        std::string standard;
        std::string build_type;
        std::string compiler;

        std::vector<std::string> global_flags;
        std::vector<std::string> global_includes;
        std::map<std::string, std::string> global_defines;

        std::vector<Target> targets;
        std::vector<Dependency> dependencies;

        std::map<std::string, std::string> variables;
    };

    using ProgressCallback = std::function<void(const std::string&, int, int)>;

    class Engine {
    public:
        Engine();
        Engine(const BuildConfig& config);
        ~Engine() = default;

        void set_config(const BuildConfig& config);
        void load_from_build_dir(const std::string& build_dir);

        void generate_build_files(const std::string& build_dir,
                                  const std::string& backend);

        int build(const std::string& target = "",
                  int jobs = 0,
                  bool verbose = false,
                  ProgressCallback progress = nullptr);

        const BuildConfig& config() const { return m_config; }

    private:
        BuildConfig m_config;
        std::string m_build_dir;

        void generate_ninja(const std::string& build_dir);
        void generate_makefile(const std::string& build_dir);

        std::vector<std::string> resolve_sources(const Target& target) const;
        std::string get_compiler() const;
        std::string get_compile_flags(const Target& target) const;
        std::string get_link_flags(const Target& target) const;
    std::string get_libs(const Target& target) const;
    std::string get_cxx_compiler() const;
    std::vector<std::string> expand_glob(const std::string& pattern) const;
    std::vector<std::string> get_build_order() const;
    bool needs_rebuild(const std::string& target_name) const;
    };

} // namespace iris::core
