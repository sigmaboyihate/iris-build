#include "interpreter.hpp"
#include "../ui/terminal.hpp"

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <fstream>

namespace fs = std::filesystem;

namespace iris::lang {

// IrisValue implementation

bool IrisValue::is_array() const {
    return std::holds_alternative<std::vector<std::shared_ptr<IrisValue>>>(data);
}

bool IrisValue::is_hash() const {
    return std::holds_alternative<std::map<std::string, std::shared_ptr<IrisValue>>>(data);
}

bool IrisValue::as_bool() const {
    if (is_bool()) return std::get<bool>(data);
    if (is_nil()) return false;
    if (is_number()) return std::get<double>(data) != 0;
    if (is_string()) return !std::get<std::string>(data).empty();
    return true;
}

double IrisValue::as_number() const {
    if (is_number()) return std::get<double>(data);
    if (is_bool()) return std::get<bool>(data) ? 1.0 : 0.0;
    if (is_string()) {
        try {
            return std::stod(std::get<std::string>(data));
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

std::string IrisValue::as_string() const {
    return to_string();
}

std::string IrisValue::to_string() const {
    if (is_nil()) return "nil";
    if (is_bool()) return std::get<bool>(data) ? "true" : "false";
    if (is_number()) {
        double d = std::get<double>(data);
        if (d == static_cast<int>(d)) {
            return std::to_string(static_cast<int>(d));
        }
        return std::to_string(d);
    }
    if (is_string()) return std::get<std::string>(data);
    if (is_array()) return "[array]";
    if (is_hash()) return "{hash}";
    return "unknown";
}

// environment implementation

Environment::Environment(std::shared_ptr<Environment> parent) 
    : m_parent(parent) {}

void Environment::define(const std::string& name, IrisValuePtr value) {
    m_values[name] = value;
}

IrisValuePtr Environment::get(const std::string& name) const {
    auto it = m_values.find(name);
    if (it != m_values.end()) {
        return it->second;
    }
    if (m_parent) {
        return m_parent->get(name);
    }
    return nullptr;
}

void Environment::set(const std::string& name, IrisValuePtr value) {
    if (m_values.count(name)) {
        m_values[name] = value;
    } else if (m_parent && m_parent->exists(name)) {
        m_parent->set(name, value);
    } else {
        m_values[name] = value;
    }
}

bool Environment::exists(const std::string& name) const {
    if (m_values.count(name)) return true;
    if (m_parent) return m_parent->exists(name);
    return false;
}

// interpreter implementation

Interpreter::Interpreter() {
    m_global_env = std::make_shared<Environment>();
    m_current_env = m_global_env;
    register_builtins();
}

void Interpreter::register_builtins() {
    // glob function find files matching a pattern
    m_native_functions["glob"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        std::vector<std::shared_ptr<IrisValue>> files;
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = files;
            return result;
        }
        
        std::string pattern = args[0]->as_string();
        fs::path base_path = fs::path(pattern).parent_path();
        if (base_path.empty()) base_path = ".";
        
        std::string glob_pattern = fs::path(pattern).filename().string();
        
        // simple glob matching - convert glob to regex
        std::string regex_pattern = glob_pattern;
        std::string final_regex;
        
        for (size_t i = 0; i < regex_pattern.size(); i++) {
            char c = regex_pattern[i];
            if (c == '*' && i + 1 < regex_pattern.size() && regex_pattern[i+1] == '*') {
                final_regex += ".*";
                i++;
            } else if (c == '*') {
                final_regex += "[^/]*";
            } else if (c == '?') {
                final_regex += ".";
            } else if (c == '.') {
                final_regex += "\\.";
            } else {
                final_regex += c;
            }
        }
        
        std::regex re;
        try {
            re = std::regex(final_regex);
        } catch (...) {
            result->data = files;
            return result;
        }
        
        if (fs::exists(base_path)) {
            for (const auto& entry : fs::recursive_directory_iterator(base_path)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, re)) {
                        auto file_val = std::make_shared<IrisValue>();
                        file_val->data = entry.path().string();
                        files.push_back(file_val);
                    }
                }
            }
        }
        
        result->data = files;
        return result;
    };
    
    // find_package function locate system packages
    m_native_functions["find_package"] = [this](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = nullptr;
            return result;
        }
        
        std::string pkg_name = args[0]->as_string();
        
        // try pkg-config first
        std::string cmd = "pkg-config --exists " + pkg_name + " 2>/dev/null";
        int ret = std::system(cmd.c_str());
        
        if (ret == 0) {
            std::map<std::string, std::shared_ptr<IrisValue>> pkg_info;
            
            auto name_val = std::make_shared<IrisValue>();
            name_val->data = pkg_name;
            pkg_info["name"] = name_val;
            
            auto found_val = std::make_shared<IrisValue>();
            found_val->data = true;
            pkg_info["found"] = found_val;
            
            result->data = pkg_info;
        } else {
            result->data = nullptr;
        }
        
        return result;
    };
    
    // find_library function
    m_native_functions["find_library"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = nullptr;
            return result;
        }
        
        std::string lib_name = args[0]->as_string();
        
        // common library paths
        std::vector<std::string> search_paths = {
            "/usr/lib",
            "/usr/local/lib",
            "/usr/lib/x86_64-linux-gnu",
            "/lib",
            "/lib64"
        };
        
        for (const auto& path : search_paths) {
            fs::path lib_path = fs::path(path) / ("lib" + lib_name + ".so");
            fs::path lib_path_a = fs::path(path) / ("lib" + lib_name + ".a");
            
            if (fs::exists(lib_path) || fs::exists(lib_path_a)) {
                std::map<std::string, std::shared_ptr<IrisValue>> lib_info;
                
                auto name_val = std::make_shared<IrisValue>();
                name_val->data = lib_name;
                lib_info["name"] = name_val;
                
                auto found_val = std::make_shared<IrisValue>();
                found_val->data = true;
                lib_info["found"] = found_val;
                
                auto path_val = std::make_shared<IrisValue>();
                path_val->data = path;
                lib_info["path"] = path_val;
                
                result->data = lib_info;
                return result;
            }
        }
        
        result->data = nullptr;
        return result;
    };
    
    // print function
    m_native_functions["print"] = [](const std::vector<IrisValuePtr>& args) {
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << args[i]->to_string();
        }
        std::cout << std::endl;
        return std::make_shared<IrisValue>();
    };
    
    // error function
    m_native_functions["error"] = [](const std::vector<IrisValuePtr>& args) {
        std::string msg = args.empty() ? "Build error" : args[0]->as_string();
        throw std::runtime_error(msg);
        return std::make_shared<IrisValue>();
    };
    
    // warning function
    m_native_functions["warning"] = [](const std::vector<IrisValuePtr>& args) {
        if (!args.empty()) {
            ui::Terminal::warning(args[0]->as_string());
        }
        return std::make_shared<IrisValue>();
    };
    
    // shell function execute shell command
    m_native_functions["shell"] = [](const std::vector<IrisValuePtr>& args) {
        if (args.empty() || !args[0]->is_string()) {
            return std::make_shared<IrisValue>();
        }
        
        std::string cmd = args[0]->as_string();
        
        // capture output
        std::array<char, 256> buffer;
        std::string output;
        
        FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
        if (pipe) {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                output += buffer.data();
            }
            pclose(pipe);
        }
        
        // remove trailing newline
        if (!output.empty() && output.back() == '\n') {
            output.pop_back();
        }
        
        auto result = std::make_shared<IrisValue>();
        result->data = output;
        return result;
    };
    
    // run function execute command and return exit code
    m_native_functions["run"] = [](const std::vector<IrisValuePtr>& args) {
        if (args.empty() || !args[0]->is_string()) {
            auto result = std::make_shared<IrisValue>();
            result->data = -1.0;
            return result;
        }
        
        int ret = std::system(args[0]->as_string().c_str());
        auto result = std::make_shared<IrisValue>();
        result->data = static_cast<double>(ret);
        return result;
    };
    
    // env function get environment variable
    m_native_functions["env"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = std::string("");
            return result;
        }
        
        const char* val = std::getenv(args[0]->as_string().c_str());
        result->data = val ? std::string(val) : std::string("");
        return result;
    };
    
    // platform function get current platform
    m_native_functions["platform"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
#if defined(_WIN32)
        result->data = std::string("windows");
#elif defined(__APPLE__)
        result->data = std::string("macos");
#elif defined(__linux__)
        result->data = std::string("linux");
#elif defined(__FreeBSD__)
        result->data = std::string("freebsd");
#else
        result->data = std::string("unix");
#endif
        return result;
    };
    
    // arch function - get current architecture
    m_native_functions["arch"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
#if defined(__x86_64__) || defined(_M_X64)
        result->data = std::string("x86_64");
#elif defined(__i386__) || defined(_M_IX86)
        result->data = std::string("x86");
#elif defined(__aarch64__) || defined(_M_ARM64)
        result->data = std::string("arm64");
#elif defined(__arm__) || defined(_M_ARM)
        result->data = std::string("arm");
#else
        result->data = std::string("unknown");
#endif
        return result;
    };
    
    // join function join array elements with separator
    m_native_functions["join"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.size() < 2 || !args[0]->is_array() || !args[1]->is_string()) {
            result->data = std::string("");
            return result;
        }
        
        auto& arr = std::get<std::vector<std::shared_ptr<IrisValue>>>(args[0]->data);
        std::string sep = args[1]->as_string();
        
        std::string joined;
        for (size_t i = 0; i < arr.size(); i++) {
            if (i > 0) joined += sep;
            joined += arr[i]->to_string();
        }
        
        result->data = joined;
        return result;
    };
    
    // split function split string into array
    m_native_functions["split"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        std::vector<std::shared_ptr<IrisValue>> parts;
        
        if (args.size() < 2 || !args[0]->is_string() || !args[1]->is_string()) {
            result->data = parts;
            return result;
        }
        
        std::string str = args[0]->as_string();
        std::string delim = args[1]->as_string();
        
        size_t pos = 0;
        while ((pos = str.find(delim)) != std::string::npos) {
            auto part = std::make_shared<IrisValue>();
            part->data = str.substr(0, pos);
            parts.push_back(part);
            str.erase(0, pos + delim.length());
        }
        
        if (!str.empty()) {
            auto part = std::make_shared<IrisValue>();
            part->data = str;
            parts.push_back(part);
        }
        
        result->data = parts;
        return result;
    };
    
    // contains function check if array contains element
    m_native_functions["contains"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        result->data = false;
        
        if (args.size() < 2 || !args[0]->is_array()) {
            return result;
        }
        
        auto& arr = std::get<std::vector<std::shared_ptr<IrisValue>>>(args[0]->data);
        std::string needle = args[1]->to_string();
        
        for (const auto& elem : arr) {
            if (elem->to_string() == needle) {
                result->data = true;
                break;
            }
        }
        
        return result;
    };
    
    // len function get length of string or array
    m_native_functions["len"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty()) {
            result->data = 0.0;
            return result;
        }
        
        if (args[0]->is_string()) {
            result->data = static_cast<double>(args[0]->as_string().length());
        } else if (args[0]->is_array()) {
            auto& arr = std::get<std::vector<std::shared_ptr<IrisValue>>>(args[0]->data);
            result->data = static_cast<double>(arr.size());
        } else {
            result->data = 0.0;
        }
        
        return result;
    };
    
    // file_exists function
    m_native_functions["file_exists"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = false;
            return result;
        }
        
        result->data = fs::exists(args[0]->as_string());
        return result;
    };
    
    // read_file function
    m_native_functions["read_file"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = std::string("");
            return result;
        }
        
        std::ifstream file(args[0]->as_string());
        if (!file.is_open()) {
            result->data = std::string("");
            return result;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        result->data = buffer.str();
        return result;
    };
    
    // write_file function
    m_native_functions["write_file"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        result->data = false;
        
        if (args.size() < 2 || !args[0]->is_string() || !args[1]->is_string()) {
            return result;
        }
        
        std::ofstream file(args[0]->as_string());
        if (file.is_open()) {
            file << args[1]->as_string();
            result->data = true;
        }
        
        return result;
    };
    
    // dirname function
    m_native_functions["dirname"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = std::string("");
            return result;
        }
        
        result->data = fs::path(args[0]->as_string()).parent_path().string();
        return result;
    };
    
    // basename function
    m_native_functions["basename"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = std::string("");
            return result;
        }
        
        result->data = fs::path(args[0]->as_string()).filename().string();
        return result;
    };
    
    // extension function
    m_native_functions["extension"] = [](const std::vector<IrisValuePtr>& args) {
        auto result = std::make_shared<IrisValue>();
        
        if (args.empty() || !args[0]->is_string()) {
            result->data = std::string("");
            return result;
        }
        
        result->data = fs::path(args[0]->as_string()).extension().string();
        return result;
    };
}

void Interpreter::set_variable(const std::string& name, const std::string& value) {
    auto val = make_value(value);
    m_global_env->define(name, val);
}

std::string Interpreter::get_variable(const std::string& name) const {
    auto val = m_global_env->get(name);
    if (val) return val->as_string();
    return "";
}

core::BuildConfig Interpreter::execute(const AST& ast) {
    m_config = core::BuildConfig();
    
    // set up built in variables
    m_global_env->define("platform", m_native_functions["platform"]({}));
    m_global_env->define("arch", m_native_functions["arch"]({}));
    
    for (const auto& stmt : ast.statements) {
        eval_statement(stmt);
    }
    
    return m_config;
}

void Interpreter::eval_statement(const StmtPtr& stmt) {
    if (auto p = std::dynamic_pointer_cast<ProjectBlock>(stmt)) {
        eval_project(p.get());
    } else if (auto t = std::dynamic_pointer_cast<TargetBlock>(stmt)) {
        eval_target(t.get());
    } else if (auto c = std::dynamic_pointer_cast<CompilerBlock>(stmt)) {
        eval_compiler(c.get());
    } else if (auto t = std::dynamic_pointer_cast<TaskBlock>(stmt)) {
        eval_task(t.get());
    } else if (auto i = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        eval_if(i.get());
    } else if (auto u = std::dynamic_pointer_cast<UnlessStatement>(stmt)) {
        eval_unless(u.get());
    } else if (auto f = std::dynamic_pointer_cast<ForLoop>(stmt)) {
        eval_for(f.get());
    } else if (auto f = std::dynamic_pointer_cast<FunctionDef>(stmt)) {
        eval_function_def(f.get());
    } else if (auto a = std::dynamic_pointer_cast<Assignment>(stmt)) {
        eval_assignment(a.get());
    } else if (auto e = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        eval_expression(e->expression);
    } else if (auto r = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        // return statements are handled in function calls
        if (r->value) {
            throw eval_expression(r->value);
        }
    }
}

void Interpreter::eval_project(const ProjectBlock* block) {
    m_config.project_name = block->name;
    
    auto prev_env = m_current_env;
    m_current_env = std::make_shared<Environment>(m_current_env);
    
    eval_block(block->body.get());
    
    // extract project settings
    if (auto version = m_current_env->get("version")) {
        m_config.version = version->as_string();
    }
    if (auto lang = m_current_env->get("lang")) {
        m_config.language = lang->as_string();
    }
    if (auto std_val = m_current_env->get("std")) {
        m_config.standard = std_val->as_string();
    }
    if (auto license = m_current_env->get("license")) {
        // store license info if needed
    }
    
    m_current_env = prev_env;
}

void Interpreter::eval_target(const TargetBlock* block) {
    core::Target target;
    target.name = block->name;
    
    if (block->target_type == "executable") {
        target.type = core::TargetType::Executable;
    } else if (block->target_type == "library" || block->target_type == "static_library") {
        target.type = core::TargetType::Library;
    } else if (block->target_type == "shared_library") {
        target.type = core::TargetType::SharedLibrary;
    } else {
        target.type = core::TargetType::Executable;
    }
    
    auto prev_env = m_current_env;
    m_current_env = std::make_shared<Environment>(m_current_env);
    
    eval_block(block->body.get());
    
    // extract target settings
    if (auto sources = m_current_env->get("sources")) {
        target.sources = value_to_string_list(sources);
    }
    if (auto includes = m_current_env->get("includes")) {
        target.includes = value_to_string_list(includes);
    }
    if (auto flags = m_current_env->get("flags")) {
        target.flags = value_to_string_list(flags);
    }
    if (auto link_flags = m_current_env->get("link_flags")) {
        target.link_flags = value_to_string_list(link_flags);
    }
    if (auto deps = m_current_env->get("deps")) {
        target.dependencies = value_to_string_list(deps);
    }
    if (auto defines = m_current_env->get("defines")) {
        auto define_list = value_to_string_list(defines);
        for (const auto& def : define_list) {
            size_t eq_pos = def.find('=');
            if (eq_pos != std::string::npos) {
                target.defines[def.substr(0, eq_pos)] = def.substr(eq_pos + 1);
            } else {
                target.defines[def] = "";
            }
        }
    }
    
    m_config.targets.push_back(target);
    m_current_env = prev_env;
}

void Interpreter::eval_compiler(const CompilerBlock* block) {
    auto prev_env = m_current_env;
    m_current_env = std::make_shared<Environment>(m_current_env);
    
    eval_block(block->body.get());
    
    // extract compiler settings
    if (auto flags = m_current_env->get("flags")) {
        auto flag_list = value_to_string_list(flags);
        m_config.global_flags.insert(m_config.global_flags.end(), 
                                      flag_list.begin(), flag_list.end());
    }
    if (auto warnings = m_current_env->get("warnings")) {
        auto warning_list = value_to_string_list(warnings);
        m_config.global_flags.insert(m_config.global_flags.end(),
                                      warning_list.begin(), warning_list.end());
    }
    if (auto cc = m_current_env->get("cc")) {
        m_config.compiler = cc->as_string();
    }
    if (auto cxx = m_current_env->get("cxx")) {
        m_config.compiler = cxx->as_string();
    }
    
    m_current_env = prev_env;
}

void Interpreter::eval_task(const TaskBlock* block) {
    // store task as a callable function
    auto task_func = [this, block](const std::vector<IrisValuePtr>& args) {
        auto prev_env = m_current_env;
        m_current_env = std::make_shared<Environment>(m_current_env);
        
        eval_block(block->body.get());
        
        m_current_env = prev_env;
        return std::make_shared<IrisValue>();
    };
    
    m_native_functions["task_" + block->name] = task_func;
    
    // also store metadata about the task
    auto task_val = std::make_shared<IrisValue>();
    task_val->data = block->name;
    m_global_env->define("__task_" + block->name, task_val);
}

void Interpreter::eval_if(const IfStatement* stmt) {
    auto condition = eval_expression(stmt->condition);
    
    if (is_truthy(condition)) {
        eval_block(stmt->then_block.get());
    } else if (stmt->else_block) {
        eval_block(stmt->else_block.get());
    }
}

void Interpreter::eval_unless(const UnlessStatement* stmt) {
    auto condition = eval_expression(stmt->condition);
    
    if (!is_truthy(condition)) {
        eval_block(stmt->body.get());
    }
}

void Interpreter::eval_for(const ForLoop* stmt) {
    auto iterable = eval_expression(stmt->iterable);
    
    if (!iterable->is_array()) {
        throw std::runtime_error("For loop requires an array");
    }
    
    auto& arr = std::get<std::vector<std::shared_ptr<IrisValue>>>(iterable->data);
    
    auto prev_env = m_current_env;
    m_current_env = std::make_shared<Environment>(m_current_env);
    
    for (const auto& elem : arr) {
        m_current_env->define(stmt->variable, elem);
        eval_block(stmt->body.get());
    }
    
    m_current_env = prev_env;
}

void Interpreter::eval_function_def(const FunctionDef* def) {
    // capture the function definition
    auto func = [this, def](const std::vector<IrisValuePtr>& args) -> IrisValuePtr {
        auto prev_env = m_current_env;
        m_current_env = std::make_shared<Environment>(m_global_env);
        
        // bind parameters
        for (size_t i = 0; i < def->parameters.size() && i < args.size(); i++) {
            m_current_env->define(def->parameters[i], args[i]);
        }
        
        // execute body
        IrisValuePtr result = std::make_shared<IrisValue>();
        try {
            eval_block(def->body.get());
        } catch (IrisValuePtr& ret_val) {
            result = ret_val;
        }
        
        m_current_env = prev_env;
        return result;
    };
    
    m_native_functions[def->name] = func;
}

void Interpreter::eval_assignment(const Assignment* stmt) {
    auto value = eval_expression(stmt->value);
    m_current_env->set(stmt->name, value);
}

void Interpreter::eval_block(const Block* block) {
    for (const auto& stmt : block->statements) {
        eval_statement(stmt);
    }
}

IrisValuePtr Interpreter::eval_expression(const ExprPtr& expr) {
    if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) {
        return make_value(str->value);
    }
    
    if (auto num = std::dynamic_pointer_cast<NumberLiteral>(expr)) {
        return make_value(num->value);
    }
    
    if (auto b = std::dynamic_pointer_cast<BoolLiteral>(expr)) {
        return make_value(b->value);
    }
    
    if (auto sym = std::dynamic_pointer_cast<Symbol>(expr)) {
        // symbols are like strings but can be used for enums
        return make_value(sym->name);
    }
    
    if (auto id = std::dynamic_pointer_cast<Identifier>(expr)) {
        auto value = m_current_env->get(id->name);
        if (value) return value;
        
        // check if its a builtin
        if (m_native_functions.count(id->name)) {
            // return a callable placeholder
            auto func_val = std::make_shared<IrisValue>();
            func_val->data = "__func:" + id->name;
            return func_val;
        }
        
        return std::make_shared<IrisValue>(); // nil
    }
    
    if (auto arr = std::dynamic_pointer_cast<ArrayLiteral>(expr)) {
        std::vector<IrisValuePtr> elements;
        for (const auto& elem : arr->elements) {
            elements.push_back(eval_expression(elem));
        }
        return make_array(elements);
    }
    
    if (auto hash = std::dynamic_pointer_cast<HashLiteral>(expr)) {
        auto result = std::make_shared<IrisValue>();
        std::map<std::string, IrisValuePtr> map;
        
        for (const auto& [key, val] : hash->pairs) {
            std::string key_str = eval_expression(key)->as_string();
            map[key_str] = eval_expression(val);
        }
        
        result->data = map;
        return result;
    }
    
    if (auto bin = std::dynamic_pointer_cast<BinaryOp>(expr)) {
        return eval_binary(bin.get());
    }
    
    if (auto un = std::dynamic_pointer_cast<UnaryOp>(expr)) {
        return eval_unary(un.get());
    }
    
    if (auto call = std::dynamic_pointer_cast<FunctionCall>(expr)) {
        return eval_call(call.get());
    }
    
    if (auto member = std::dynamic_pointer_cast<MemberAccess>(expr)) {
        return eval_member_access(member.get());
    }
    
    if (auto index = std::dynamic_pointer_cast<IndexAccess>(expr)) {
        return eval_index_access(index.get());
    }
    
    return std::make_shared<IrisValue>(); // nil
}

IrisValuePtr Interpreter::eval_binary(const BinaryOp* expr) {
    auto left = eval_expression(expr->left);
    auto right = eval_expression(expr->right);
    
    // string concatenation
    if (expr->op == "+" && (left->is_string() || right->is_string())) {
        return make_value(left->as_string() + right->as_string());
    }
    
    // numeric operations
    if (expr->op == "+") {
        return make_value(left->as_number() + right->as_number());
    }
    if (expr->op == "-") {
        return make_value(left->as_number() - right->as_number());
    }
    if (expr->op == "*") {
        return make_value(left->as_number() * right->as_number());
    }
    if (expr->op == "/") {
        double r = right->as_number();
        if (r == 0) throw std::runtime_error("Division by zero");
        return make_value(left->as_number() / r);
    }
    if (expr->op == "%") {
        return make_value(static_cast<double>(
            static_cast<int>(left->as_number()) % static_cast<int>(right->as_number())
        ));
    }
    
    // comparison
    if (expr->op == "==" || expr->op == "!=") {
        bool eq = false;
        if (left->is_string() && right->is_string()) {
            eq = left->as_string() == right->as_string();
        } else if (left->is_number() && right->is_number()) {
            eq = left->as_number() == right->as_number();
        } else if (left->is_bool() && right->is_bool()) {
            eq = left->as_bool() == right->as_bool();
        } else {
            eq = left->to_string() == right->to_string();
        }
        return make_value(expr->op == "==" ? eq : !eq);
    }
    
    if (expr->op == "<") {
        return make_value(left->as_number() < right->as_number());
    }
    if (expr->op == ">") {
        return make_value(left->as_number() > right->as_number());
    }
    if (expr->op == "<=") {
        return make_value(left->as_number() <= right->as_number());
    }
    if (expr->op == ">=") {
        return make_value(left->as_number() >= right->as_number());
    }
    
    // logical
    if (expr->op == "and") {
        return make_value(is_truthy(left) && is_truthy(right));
    }
    if (expr->op == "or") {
        return make_value(is_truthy(left) || is_truthy(right));
    }
    
    return std::make_shared<IrisValue>();
}

IrisValuePtr Interpreter::eval_unary(const UnaryOp* expr) {
    auto operand = eval_expression(expr->operand);
    
    if (expr->op == "-") {
        return make_value(-operand->as_number());
    }
    if (expr->op == "not" || expr->op == "!") {
        return make_value(!is_truthy(operand));
    }
    
    return operand;
}

IrisValuePtr Interpreter::eval_call(const FunctionCall* expr) {
    // evaluate arguments
    std::vector<IrisValuePtr> args;
    for (const auto& arg : expr->arguments) {
        args.push_back(eval_expression(arg));
    }
    
    // check native functions
    auto it = m_native_functions.find(expr->name);
    if (it != m_native_functions.end()) {
        return it->second(args);
    }
    
    throw std::runtime_error("Unknown function: " + expr->name);
}

IrisValuePtr Interpreter::eval_member_access(const MemberAccess* expr) {
    auto object = eval_expression(expr->object);
    
    if (object->is_hash()) {
        auto& map = std::get<std::map<std::string, IrisValuePtr>>(object->data);
        auto it = map.find(expr->member);
        if (it != map.end()) {
            return it->second;
        }
    }
    
    // array methods
    if (object->is_array()) {
        auto& arr = std::get<std::vector<IrisValuePtr>>(object->data);
        
        if (expr->member == "length" || expr->member == "size") {
            return make_value(static_cast<double>(arr.size()));
        }
        if (expr->member == "empty") {
            return make_value(arr.empty());
        }
        if (expr->member == "first" && !arr.empty()) {
            return arr.front();
        }
        if (expr->member == "last" && !arr.empty()) {
            return arr.back();
        }
    }
    
    // string methods
    if (object->is_string()) {
        std::string str = object->as_string();
        
        if (expr->member == "length" || expr->member == "size") {
            return make_value(static_cast<double>(str.length()));
        }
        if (expr->member == "empty") {
            return make_value(str.empty());
        }
        if (expr->member == "upper") {
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            return make_value(str);
        }
        if (expr->member == "lower") {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            return make_value(str);
        }
    }
    
    return std::make_shared<IrisValue>();
}

IrisValuePtr Interpreter::eval_index_access(const IndexAccess* expr) {
    auto object = eval_expression(expr->object);
    auto index = eval_expression(expr->index);
    
    if (object->is_array() && index->is_number()) {
        auto& arr = std::get<std::vector<IrisValuePtr>>(object->data);
        int idx = static_cast<int>(index->as_number());
        
        if (idx < 0) idx = static_cast<int>(arr.size()) + idx;
        
        if (idx >= 0 && static_cast<size_t>(idx) < arr.size()) {
            return arr[idx];
        }
    }
    
    if (object->is_hash() && index->is_string()) {
        auto& map = std::get<std::map<std::string, IrisValuePtr>>(object->data);
        auto it = map.find(index->as_string());
        if (it != map.end()) {
            return it->second;
        }
    }
    
    if (object->is_string() && index->is_number()) {
        std::string str = object->as_string();
        int idx = static_cast<int>(index->as_number());
        
        if (idx < 0) idx = static_cast<int>(str.length()) + idx;
        
        if (idx >= 0 && static_cast<size_t>(idx) < str.length()) {
            return make_value(std::string(1, str[idx]));
        }
    }
    
    return std::make_shared<IrisValue>();
}

IrisValuePtr Interpreter::make_value(std::nullptr_t) {
    return std::make_shared<IrisValue>();
}

IrisValuePtr Interpreter::make_value(bool value) {
    auto v = std::make_shared<IrisValue>();
    v->data = value;
    return v;
}

IrisValuePtr Interpreter::make_value(double value) {
    auto v = std::make_shared<IrisValue>();
    v->data = value;
    return v;
}

IrisValuePtr Interpreter::make_value(const std::string& value) {
    auto v = std::make_shared<IrisValue>();
    v->data = value;
    return v;
}

IrisValuePtr Interpreter::make_array(const std::vector<IrisValuePtr>& values) {
    auto v = std::make_shared<IrisValue>();
    v->data = values;
    return v;
}

bool Interpreter::is_truthy(IrisValuePtr value) const {
    if (!value) return false;
    return value->as_bool();
}

std::vector<std::string> Interpreter::value_to_string_list(IrisValuePtr value) const {
    std::vector<std::string> result;
    
    if (!value) return result;
    
    if (value->is_array()) {
        auto& arr = std::get<std::vector<IrisValuePtr>>(value->data);
        for (const auto& elem : arr) {
            result.push_back(elem->to_string());
        }
    } else if (value->is_string()) {
        result.push_back(value->as_string());
    }
    
    return result;
}

} // namespace iris::lang
