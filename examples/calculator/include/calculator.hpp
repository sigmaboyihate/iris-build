#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <stdexcept>
#include <functional>
#include <map>
#include <cmath>

namespace calc {

// Token types for lexer
enum class TokenType {
    Number,
    Plus,
    Minus,
    Multiply,
    Divide,
    Power,
    Modulo,
    LeftParen,
    RightParen,
    Identifier,
    Equals,
    Comma,
    End,
    Invalid
};

struct Token {
    TokenType type;
    std::string value;
    int position;
    
    Token(TokenType t, std::string v, int pos) 
        : type(t), value(std::move(v)), position(pos) {}
};

// Lexer class
class Lexer {
public:
    explicit Lexer(const std::string& input);
    std::vector<Token> tokenize();
    
private:
    std::string m_input;
    size_t m_pos = 0;
    
    char current() const;
    char peek(int offset = 1) const;
    void advance();
    void skip_whitespace();
    Token read_number();
    Token read_identifier();
};

// AST Node types
struct NumberNode;
struct BinaryOpNode;
struct UnaryOpNode;
struct VariableNode;
struct FunctionCallNode;
struct AssignmentNode;

using ASTNode = std::variant<
    std::shared_ptr<NumberNode>,
    std::shared_ptr<BinaryOpNode>,
    std::shared_ptr<UnaryOpNode>,
    std::shared_ptr<VariableNode>,
    std::shared_ptr<FunctionCallNode>,
    std::shared_ptr<AssignmentNode>
>;

struct NumberNode {
    double value;
    explicit NumberNode(double v) : value(v) {}
};

struct BinaryOpNode {
    char op;
    ASTNode left;
    ASTNode right;
    BinaryOpNode(char o, ASTNode l, ASTNode r) 
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

struct UnaryOpNode {
    char op;
    ASTNode operand;
    UnaryOpNode(char o, ASTNode n) : op(o), operand(std::move(n)) {}
};

struct VariableNode {
    std::string name;
    explicit VariableNode(std::string n) : name(std::move(n)) {}
};

struct FunctionCallNode {
    std::string name;
    std::vector<ASTNode> arguments;
    FunctionCallNode(std::string n, std::vector<ASTNode> args)
        : name(std::move(n)), arguments(std::move(args)) {}
};

struct AssignmentNode {
    std::string name;
    ASTNode value;
    AssignmentNode(std::string n, ASTNode v)
        : name(std::move(n)), value(std::move(v)) {}
};

// Parser class
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    ASTNode parse();
    
private:
    std::vector<Token> m_tokens;
    size_t m_pos = 0;
    
    const Token& current() const;
    const Token& peek(int offset = 1) const;
    void advance();
    bool match(TokenType type);
    void expect(TokenType type, const std::string& message);
    
    ASTNode parse_expression();
    ASTNode parse_assignment();
    ASTNode parse_additive();
    ASTNode parse_multiplicative();
    ASTNode parse_power();
    ASTNode parse_unary();
    ASTNode parse_primary();
    ASTNode parse_function_call(const std::string& name);
};

// Evaluator class
class Evaluator {
public:
    using Function = std::function<double(const std::vector<double>&)>;
    
    Evaluator();
    
    double evaluate(const ASTNode& node);
    void set_variable(const std::string& name, double value);
    double get_variable(const std::string& name) const;
    void register_function(const std::string& name, Function func);
    void clear_variables();
    
    const std::map<std::string, double>& variables() const { return m_variables; }
    
private:
    std::map<std::string, double> m_variables;
    std::map<std::string, Function> m_functions;
    
    void register_builtin_functions();
};

// Calculator class (main interface)
class Calculator {
public:
    Calculator();
    
    double calculate(const std::string& expression);
    void set_variable(const std::string& name, double value);
    double get_variable(const std::string& name) const;
    void clear();
    
    std::string get_last_error() const { return m_last_error; }
    bool has_error() const { return !m_last_error.empty(); }
    
private:
    Evaluator m_evaluator;
    std::string m_last_error;
};

// Exception classes
class CalcError : public std::runtime_error {
public:
    explicit CalcError(const std::string& msg) : std::runtime_error(msg) {}
};

class LexerError : public CalcError {
public:
    LexerError(const std::string& msg, int pos) 
        : CalcError("Lexer error at position " + std::to_string(pos) + ": " + msg) {}
};

class ParserError : public CalcError {
public:
    ParserError(const std::string& msg, int pos)
        : CalcError("Parser error at position " + std::to_string(pos) + ": " + msg) {}
};

class EvalError : public CalcError {
public:
    explicit EvalError(const std::string& msg) : CalcError("Evaluation error: " + msg) {}
};

} // namespace calc
