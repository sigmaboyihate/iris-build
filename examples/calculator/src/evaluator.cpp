#include "calculator.hpp"

namespace calc {

Evaluator::Evaluator() {
    register_builtin_functions();
}

void Evaluator::register_builtin_functions() {
    // Basic math functions
    m_functions["abs"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("abs() requires 1 argument");
        return std::abs(args[0]);
    };
    
    m_functions["sqrt"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("sqrt() requires 1 argument");
        if (args[0] < 0) throw EvalError("sqrt() of negative number");
        return std::sqrt(args[0]);
    };
    
    m_functions["pow"] = [](const std::vector<double>& args) {
        if (args.size() != 2) throw EvalError("pow() requires 2 arguments");
        return std::pow(args[0], args[1]);
    };
    
    m_functions["exp"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("exp() requires 1 argument");
        return std::exp(args[0]);
    };
    
    m_functions["log"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("log() requires 1 argument");
        if (args[0] <= 0) throw EvalError("log() of non-positive number");
        return std::log(args[0]);
    };
    
    m_functions["log10"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("log10() requires 1 argument");
        if (args[0] <= 0) throw EvalError("log10() of non-positive number");
        return std::log10(args[0]);
    };
    
    m_functions["log2"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("log2() requires 1 argument");
        if (args[0] <= 0) throw EvalError("log2() of non-positive number");
        return std::log2(args[0]);
    };
    
    // Trigonometric functions
    m_functions["sin"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("sin() requires 1 argument");
        return std::sin(args[0]);
    };
    
    m_functions["cos"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("cos() requires 1 argument");
        return std::cos(args[0]);
    };
    
    m_functions["tan"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("tan() requires 1 argument");
        return std::tan(args[0]);
    };
    
    m_functions["asin"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("asin() requires 1 argument");
        if (args[0] < -1 || args[0] > 1) throw EvalError("asin() argument out of range");
        return std::asin(args[0]);
    };
    
    m_functions["acos"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("acos() requires 1 argument");
        if (args[0] < -1 || args[0] > 1) throw EvalError("acos() argument out of range");
        return std::acos(args[0]);
    };
    
    m_functions["atan"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("atan() requires 1 argument");
        return std::atan(args[0]);
    };
    
    m_functions["atan2"] = [](const std::vector<double>& args) {
        if (args.size() != 2) throw EvalError("atan2() requires 2 arguments");
        return std::atan2(args[0], args[1]);
    };
    
    // Hyperbolic functions
    m_functions["sinh"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("sinh() requires 1 argument");
        return std::sinh(args[0]);
    };
    
    m_functions["cosh"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("cosh() requires 1 argument");
        return std::cosh(args[0]);
    };
    
    m_functions["tanh"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("tanh() requires 1 argument");
        return std::tanh(args[0]);
    };
    
    // Rounding functions
    m_functions["floor"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("floor() requires 1 argument");
        return std::floor(args[0]);
    };
    
    m_functions["ceil"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("ceil() requires 1 argument");
        return std::ceil(args[0]);
    };
    
    m_functions["round"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("round() requires 1 argument");
        return std::round(args[0]);
    };
    
    m_functions["trunc"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw EvalError("trunc() requires 1 argument");
        return std::trunc(args[0]);
    };
    
    // Min/Max
    m_functions["min"] = [](const std::vector<double>& args) {
        if (args.empty()) throw EvalError("min() requires at least 1 argument");
        double result = args[0];
        for (size_t i = 1; i < args.size(); i++) {
            result = std::min(result, args[i]);
        }
        return result;
    };
    
    m_functions["max"] = [](const std::vector<double>& args) {
        if (args.empty()) throw EvalError("max() requires at least 1 argument");
        double result = args[0];
        for (size_t i = 1; i < args.size(); i++) {
            result = std::max(result, args[i]);
        }
        return result;
    };
    
    // Constants as functions
    m_functions["pi"] = [](const std::vector<double>& args) {
        (void)args;
        return 3.14159265358979323846;
    };
    
    m_functions["e"] = [](const std::vector<double>& args) {
        (void)args;
        return 2.71828182845904523536;
    };
    
    // Set common constants as variables
    m_variables["pi"] = 3.14159265358979323846;
    m_variables["e"] = 2.71828182845904523536;
    m_variables["tau"] = 6.28318530717958647692;
    m_variables["phi"] = 1.61803398874989484820;
}

void Evaluator::set_variable(const std::string& name, double value) {
    m_variables[name] = value;
}

double Evaluator::get_variable(const std::string& name) const {
    auto it = m_variables.find(name);
    if (it == m_variables.end()) {
        throw EvalError("Undefined variable: " + name);
    }
    return it->second;
}

void Evaluator::register_function(const std::string& name, Function func) {
    m_functions[name] = std::move(func);
}

void Evaluator::clear_variables() {
    m_variables.clear();
    m_variables["pi"] = 3.14159265358979323846;
    m_variables["e"] = 2.71828182845904523536;
    m_variables["tau"] = 6.28318530717958647692;
    m_variables["phi"] = 1.61803398874989484820;
}

double Evaluator::evaluate(const ASTNode& node) {
    return std::visit([this](const auto& n) -> double {
        using T = std::decay_t<decltype(n)>;
        
        if constexpr (std::is_same_v<T, std::shared_ptr<NumberNode>>) {
            return n->value;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<BinaryOpNode>>) {
            double left = evaluate(n->left);
            double right = evaluate(n->right);
            
            switch (n->op) {
                case '+': return left + right;
                case '-': return left - right;
                case '*': return left * right;
                case '/':
                    if (right == 0) throw EvalError("Division by zero");
                    return left / right;
                case '%':
                    if (right == 0) throw EvalError("Modulo by zero");
                    return std::fmod(left, right);
                case '^': return std::pow(left, right);
                default:
                    throw EvalError("Unknown operator");
            }
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<UnaryOpNode>>) {
            double operand = evaluate(n->operand);
            
            switch (n->op) {
                case '-': return -operand;
                case '+': return operand;
                default:
                    throw EvalError("Unknown unary operator");
            }
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<VariableNode>>) {
            return get_variable(n->name);
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<FunctionCallNode>>) {
            auto it = m_functions.find(n->name);
            if (it == m_functions.end()) {
                throw EvalError("Unknown function: " + n->name);
            }
            
            std::vector<double> args;
            args.reserve(n->arguments.size());
            for (const auto& arg : n->arguments) {
                args.push_back(evaluate(arg));
            }
            
            return it->second(args);
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<AssignmentNode>>) {
            double value = evaluate(n->value);
            m_variables[n->name] = value;
            return value;
        }
        else {
            throw EvalError("Unknown node type");
        }
    }, node);
}

} // namespace calc
