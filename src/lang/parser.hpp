#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include <stdexcept>

#include <string>
#include <vector>
#include <memory>

namespace iris::lang {

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, int line, int column)
        : std::runtime_error(message), m_line(line), m_column(column) {}
    
    int line() const { return m_line; }
    int column() const { return m_column; }

private:
    int m_line;
    int m_column;
};

class Parser {
public:
    Parser();
    
    AST parse(const std::string& source);
    AST parse_file(const std::string& filename);

private:
    std::vector<Token> m_tokens;
    size_t m_current = 0;

    // Parsing methods
    StmtPtr parse_statement();
    StmtPtr parse_project_block();
    StmtPtr parse_target_block(const std::string& type);
    StmtPtr parse_compiler_block();
    StmtPtr parse_dependency_block();
    StmtPtr parse_task_block();
    StmtPtr parse_if_statement();
    StmtPtr parse_unless_statement();
    StmtPtr parse_for_loop();
    StmtPtr parse_function_def();
    StmtPtr parse_assignment_or_expression();
    
    std::shared_ptr<Block> parse_block();
    
    ExprPtr parse_expression();
    ExprPtr parse_or();
    ExprPtr parse_and();
    ExprPtr parse_equality();
    ExprPtr parse_comparison();
    ExprPtr parse_term();
    ExprPtr parse_factor();
    ExprPtr parse_unary();
    ExprPtr parse_call();
    ExprPtr parse_primary();
    ExprPtr parse_array();
    ExprPtr parse_hash();
    
    // Token helpers
    Token current_token() const;
    Token previous_token() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    void skip_newlines();
    bool is_at_end() const;
    
    ParseError error(const std::string& message);
};

} // namespace iris::lang