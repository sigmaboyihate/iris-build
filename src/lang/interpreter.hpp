#pragma once

#include "ast.hpp"
#include "../core/engine.hpp"

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <functional>

namespace iris::lang {

// value types in the interpreter
using Value = std::variant<
    std::nullptr_t,
    bool,
    double,
    std::string,
    std::vector<std::shared_ptr<struct IrisValue>>,
    std::map<std::string, std::shared_ptr<struct IrisValue>>
>;

struct IrisValue {
    Value data;
    
    bool is_nil() const { return std::holds_alternative<std::nullptr_t>(data); }
    bool is_bool() const { return std::holds_alternative<bool>(data); }
    bool is_number() const { return std::holds_alternative<double>(data); }
    bool is_string() const { return std::holds_alternative<std::string>(data); }
    bool is_array() const;
    bool is_hash() const;
    
    bool as_bool() const;
    double as_number() const;
    std::string as_string() const;
    
    std::string to_string() const;
};

using IrisValuePtr = std::shared_ptr<IrisValue>;
using NativeFunction = std::function<IrisValuePtr(const std::vector<IrisValuePtr>&)>;

class Environment {
public:
    Environment(std::shared_ptr<Environment> parent = nullptr);
    
    void define(const std::string& name, IrisValuePtr value);
    IrisValuePtr get(const std::string& name) const;
    void set(const std::string& name, IrisValuePtr value);
    bool exists(const std::string& name) const;
    
private:
    std::map<std::string, IrisValuePtr> m_values;
    std::shared_ptr<Environment> m_parent;
};

class Interpreter {
public:
    Interpreter();
    
    core::BuildConfig execute(const AST& ast);
    
    void set_variable(const std::string& name, const std::string& value);
    std::string get_variable(const std::string& name) const;
    
private:
    std::shared_ptr<Environment> m_global_env;
    std::shared_ptr<Environment> m_current_env;
    core::BuildConfig m_config;
    
    // builti n functions
    std::map<std::string, NativeFunction> m_native_functions;
    
    void register_builtins();
    
    // statement evaluation
    void eval_statement(const StmtPtr& stmt);
    void eval_project(const ProjectBlock* block);
    void eval_target(const TargetBlock* block);
    void eval_compiler(const CompilerBlock* block);
    void eval_task(const TaskBlock* block);
    void eval_if(const IfStatement* stmt);
    void eval_unless(const UnlessStatement* stmt);
    void eval_for(const ForLoop* stmt);
    void eval_function_def(const FunctionDef* def);
    void eval_assignment(const Assignment* stmt);
    void eval_block(const Block* block);
    
    // expression evaluation
    IrisValuePtr eval_expression(const ExprPtr& expr);
    IrisValuePtr eval_binary(const BinaryOp* expr);
    IrisValuePtr eval_unary(const UnaryOp* expr);
    IrisValuePtr eval_call(const FunctionCall* expr);
    IrisValuePtr eval_member_access(const MemberAccess* expr);
    IrisValuePtr eval_index_access(const IndexAccess* expr);
    
    // helpers
    IrisValuePtr make_value(std::nullptr_t);
    IrisValuePtr make_value(bool value);
    IrisValuePtr make_value(double value);
    IrisValuePtr make_value(const std::string& value);
    IrisValuePtr make_array(const std::vector<IrisValuePtr>& values);
    
    bool is_truthy(IrisValuePtr value) const;
    std::vector<std::string> value_to_string_list(IrisValuePtr value) const;
};

} // namespace iris::lang
