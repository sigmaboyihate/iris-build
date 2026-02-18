#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <map>

namespace iris::lang {

// Forward declarations
struct ASTNode;
struct Expression;
struct Statement;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ExprPtr = std::shared_ptr<Expression>;
using StmtPtr = std::shared_ptr<Statement>;

// ============================================================================
// Expressions
// ============================================================================

struct Expression {
    virtual ~Expression() = default;
    virtual std::string type_name() const = 0;
};

struct StringLiteral : Expression {
    std::string value;
    std::string type_name() const override { return "StringLiteral"; }
};

struct NumberLiteral : Expression {
    double value;
    bool is_integer;
    std::string type_name() const override { return "NumberLiteral"; }
};

struct BoolLiteral : Expression {
    bool value;
    std::string type_name() const override { return "BoolLiteral"; }
};

struct Symbol : Expression {
    std::string name;  // Without the leading colon
    std::string type_name() const override { return "Symbol"; }
};

struct Identifier : Expression {
    std::string name;
    std::string type_name() const override { return "Identifier"; }
};

struct ArrayLiteral : Expression {
    std::vector<ExprPtr> elements;
    std::string type_name() const override { return "ArrayLiteral"; }
};

struct HashLiteral : Expression {
    std::vector<std::pair<ExprPtr, ExprPtr>> pairs;
    std::string type_name() const override { return "HashLiteral"; }
};

struct BinaryOp : Expression {
    std::string op;  // +, -, *, /, ==, !=, <, >, <=, >=, and, or
    ExprPtr left;
    ExprPtr right;
    std::string type_name() const override { return "BinaryOp"; }
};

struct UnaryOp : Expression {
    std::string op;  // -, not, !
    ExprPtr operand;
    std::string type_name() const override { return "UnaryOp"; }
};

struct FunctionCall : Expression {
    std::string name;
    std::vector<ExprPtr> arguments;
    std::string type_name() const override { return "FunctionCall"; }
};

struct MemberAccess : Expression {
    ExprPtr object;
    std::string member;
    std::string type_name() const override { return "MemberAccess"; }
};

struct IndexAccess : Expression {
    ExprPtr object;
    ExprPtr index;
    std::string type_name() const override { return "IndexAccess"; }
};

struct StringInterpolation : Expression {
    std::vector<std::variant<std::string, ExprPtr>> parts;
    std::string type_name() const override { return "StringInterpolation"; }
};

// ============================================================================
// Statements
// ============================================================================

struct Statement {
    virtual ~Statement() = default;
    virtual std::string type_name() const = 0;
};

struct Assignment : Statement {
    std::string name;
    ExprPtr value;
    std::string type_name() const override { return "Assignment"; }
};

struct Block : Statement {
    std::vector<StmtPtr> statements;
    std::string type_name() const override { return "Block"; }
};

struct ProjectBlock : Statement {
    std::string name;
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "ProjectBlock"; }
};

struct TargetBlock : Statement {
    std::string name;
    std::string target_type;  // executable, library, shared_library, etc.
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "TargetBlock"; }
};

struct CompilerBlock : Statement {
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "CompilerBlock"; }
};

struct DependencyBlock : Statement {
    std::string name;
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "DependencyBlock"; }
};

struct TaskBlock : Statement {
    std::string name;  // Can be a string or symbol
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "TaskBlock"; }
};

struct IfStatement : Statement {
    ExprPtr condition;
    std::shared_ptr<Block> then_block;
    std::shared_ptr<Block> else_block;  // Optional
    std::string type_name() const override { return "IfStatement"; }
};

struct UnlessStatement : Statement {
    ExprPtr condition;
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "UnlessStatement"; }
};

struct ForLoop : Statement {
    std::string variable;
    ExprPtr iterable;
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "ForLoop"; }
};

struct FunctionDef : Statement {
    std::string name;
    std::vector<std::string> parameters;
    std::shared_ptr<Block> body;
    std::string type_name() const override { return "FunctionDef"; }
};

struct ExpressionStatement : Statement {
    ExprPtr expression;
    std::string type_name() const override { return "ExpressionStatement"; }
};

struct ReturnStatement : Statement {
    ExprPtr value;  // Optional
    std::string type_name() const override { return "ReturnStatement"; }
};

// ============================================================================
// AST Root
// ============================================================================

struct AST {
    std::vector<StmtPtr> statements;
};

} // namespace iris::lang