#include "parser.hpp"
#include <fstream>
#include <sstream>

namespace iris::lang {

Parser::Parser() = default;

AST Parser::parse(const std::string& source) {
    Lexer lexer(source);
    m_tokens = lexer.tokenize();
    m_current = 0;
    
    AST ast;
    
    while (!is_at_end()) {
        skip_newlines();
        if (!is_at_end()) {
            ast.statements.push_back(parse_statement());
        }
    }
    
    return ast;
}

AST Parser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse(buffer.str());
}

StmtPtr Parser::parse_statement() {
    skip_newlines();
    
    if (match(TokenType::PROJECT)) {
        return parse_project_block();
    }
    if (match(TokenType::EXECUTABLE)) {
        return parse_target_block("executable");
    }
    if (match(TokenType::LIBRARY)) {
        return parse_target_block("library");
    }
    if (match(TokenType::SHARED_LIBRARY)) {
        return parse_target_block("shared_library");
    }
    if (match(TokenType::STATIC_LIBRARY)) {
        return parse_target_block("static_library");
    }
    if (match(TokenType::COMPILER)) {
        return parse_compiler_block();
    }
    if (match(TokenType::DEPENDENCY)) {
        return parse_dependency_block();
    }
    if (match(TokenType::TASK)) {
        return parse_task_block();
    }
    if (match(TokenType::IF)) {
        return parse_if_statement();
    }
    if (match(TokenType::UNLESS)) {
        return parse_unless_statement();
    }
    if (match(TokenType::FOR)) {
        return parse_for_loop();
    }
    if (match(TokenType::FN)) {
        return parse_function_def();
    }
    if (match(TokenType::RETURN)) {
        auto stmt = std::make_shared<ReturnStatement>();
        if (!check(TokenType::NEWLINE) && !check(TokenType::END)) {
            stmt->value = parse_expression();
        }
        return stmt;
    }
    
    return parse_assignment_or_expression();
}

StmtPtr Parser::parse_project_block() {
    auto stmt = std::make_shared<ProjectBlock>();
    
    // project name (string)
    Token name_token = consume(TokenType::STRING, "Expected project name");
    stmt->name = name_token.value;
    
    // do
    consume(TokenType::DO, "Expected 'do' after project name");
    
    // block body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close project block");
    
    return stmt;
}

StmtPtr Parser::parse_target_block(const std::string& type) {
    auto stmt = std::make_shared<TargetBlock>();
    stmt->target_type = type;
    
    // target name (string)
    Token name_token = consume(TokenType::STRING, "Expected target name");
    stmt->name = name_token.value;
    
    // do
    consume(TokenType::DO, "Expected 'do' after target name");
    
    // block body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close target block");
    
    return stmt;
}

StmtPtr Parser::parse_compiler_block() {
    auto stmt = std::make_shared<CompilerBlock>();
    
    // do
    consume(TokenType::DO, "Expected 'do' after 'compiler'");
    
    // block body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close compiler block");
    
    return stmt;
}

StmtPtr Parser::parse_dependency_block() {
    auto stmt = std::make_shared<DependencyBlock>();
    
    // dependency name
    if (check(TokenType::STRING)) {
        stmt->name = advance().value;
    } else if (check(TokenType::IDENTIFIER)) {
        stmt->name = advance().value;
    } else {
        throw error("Expected dependency name");
    }
    
    // do
    consume(TokenType::DO, "Expected 'do' after dependency name");
    
    // block body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close dependency block");
    
    return stmt;
}

StmtPtr Parser::parse_task_block() {
    auto stmt = std::make_shared<TaskBlock>();
    
    // task name (symbol or string)
    if (check(TokenType::SYMBOL)) {
        stmt->name = advance().value;
    } else if (check(TokenType::STRING)) {
        stmt->name = advance().value;
    } else {
        throw error("Expected task name");
    }
    
    // do
    consume(TokenType::DO, "Expected 'do' after task name");
    
    // block body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close task block");
    
    return stmt;
}

StmtPtr Parser::parse_if_statement() {
    auto stmt = std::make_shared<IfStatement>();
    
    // condition
    stmt->condition = parse_expression();
    
    // do
    consume(TokenType::DO, "Expected 'do' after if condition");
    
    // then block
    stmt->then_block = parse_block();
    
    // optional else
    if (match(TokenType::ELSE)) {
        if (match(TokenType::IF)) {
            // else if create a new block containing just the if statement
            auto else_block = std::make_shared<Block>();
            else_block->statements.push_back(parse_if_statement());
            stmt->else_block = else_block;
        } else {
            stmt->else_block = parse_block();
        }
    }
    
    // end (only if no else if chain)
    if (!stmt->else_block || 
        stmt->else_block->statements.empty() ||
        stmt->else_block->statements[0]->type_name() != "IfStatement") {
        consume(TokenType::END, "Expected 'end' to close if statement");
    }
    
    return stmt;
}

StmtPtr Parser::parse_unless_statement() {
    auto stmt = std::make_shared<UnlessStatement>();
    
    // condition
    stmt->condition = parse_expression();
    
    // do
    consume(TokenType::DO, "Expected 'do' after unless condition");
    
    // body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close unless statement");
    
    return stmt;
}

StmtPtr Parser::parse_for_loop() {
    auto stmt = std::make_shared<ForLoop>();
    
    // variable name
    Token var = consume(TokenType::IDENTIFIER, "Expected variable name");
    stmt->variable = var.value;
    
    // in
    consume(TokenType::IN, "Expected 'in' in for loop");
    
    // iterable expression
    stmt->iterable = parse_expression();
    
    // do
    consume(TokenType::DO, "Expected 'do' after for loop header");
    
    // body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close for loop");
    
    return stmt;
}

StmtPtr Parser::parse_function_def() {
    auto stmt = std::make_shared<FunctionDef>();
    
    // function name
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    stmt->name = name.value;
    
    // parameters
    consume(TokenType::LPAREN, "Expected '(' after function name");
    
    if (!check(TokenType::RPAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expected parameter name");
            stmt->parameters.push_back(param.value);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RPAREN, "Expected ')' after parameters");
    
    // do
    consume(TokenType::DO, "Expected 'do' after function parameters");
    
    // body
    stmt->body = parse_block();
    
    // end
    consume(TokenType::END, "Expected 'end' to close function definition");
    
    return stmt;
}

StmtPtr Parser::parse_assignment_or_expression() {
    // check if this is an assignment
    if (check(TokenType::IDENTIFIER)) {
        Token id = current_token();
        advance();
        
        if (match(TokenType::EQ)) {
            auto stmt = std::make_shared<Assignment>();
            stmt->name = id.value;
            stmt->value = parse_expression();
            return stmt;
        } else if (match(TokenType::PLUSEQ)) {
            auto stmt = std::make_shared<Assignment>();
            stmt->name = id.value;
            auto left = std::make_shared<Identifier>();
            left->name = id.value;
            auto right = parse_expression();
            auto binary = std::make_shared<BinaryOp>();
            binary->op = "+";
            binary->left = left;
            binary->right = right;
            stmt->value = binary;
            return stmt;
        }
        
        // not an assignment, backtrack and parse as expression
        m_current--;
    }
    
    // expression statement
    auto stmt = std::make_shared<ExpressionStatement>();
    stmt->expression = parse_expression();
    return stmt;
}

std::shared_ptr<Block> Parser::parse_block() {
    auto block = std::make_shared<Block>();
    
    skip_newlines();
    
    while (!check(TokenType::END) && !check(TokenType::ELSE) && !is_at_end()) {
        block->statements.push_back(parse_statement());
        skip_newlines();
    }
    
    return block;
}

ExprPtr Parser::parse_expression() {
    return parse_or();
}

ExprPtr Parser::parse_or() {
    auto left = parse_and();
    
    while (match(TokenType::OR)) {
        auto expr = std::make_shared<BinaryOp>();
        expr->op = "or";
        expr->left = left;
        expr->right = parse_and();
        left = expr;
    }
    
    return left;
}

ExprPtr Parser::parse_and() {
    auto left = parse_equality();
    
    while (match(TokenType::AND)) {
        auto expr = std::make_shared<BinaryOp>();
        expr->op = "and";
        expr->left = left;
        expr->right = parse_equality();
        left = expr;
    }
    
    return left;
}

ExprPtr Parser::parse_equality() {
    auto left = parse_comparison();
    
    while (match({TokenType::EQEQ, TokenType::NEQ})) {
        auto expr = std::make_shared<BinaryOp>();
        expr->op = previous_token().value;
        expr->left = left;
        expr->right = parse_comparison();
        left = expr;
    }
    
    return left;
}

ExprPtr Parser::parse_comparison() {
    auto left = parse_term();
    
    while (match({TokenType::LT, TokenType::GT, TokenType::LTE, TokenType::GTE})) {
        auto expr = std::make_shared<BinaryOp>();
        Token op = previous_token();
        switch (op.type) {
            case TokenType::LT: expr->op = "<"; break;
            case TokenType::GT: expr->op = ">"; break;
            case TokenType::LTE: expr->op = "<="; break;
            case TokenType::GTE: expr->op = ">="; break;
            default: break;
        }
        expr->left = left;
        expr->right = parse_term();
        left = expr;
    }
    
    return left;
}

ExprPtr Parser::parse_term() {
    auto left = parse_factor();
    
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        auto expr = std::make_shared<BinaryOp>();
        expr->op = previous_token().type == TokenType::PLUS ? "+" : "-";
        expr->left = left;
        expr->right = parse_factor();
        left = expr;
    }
    
    return left;
}

ExprPtr Parser::parse_factor() {
    auto left = parse_unary();
    
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        auto expr = std::make_shared<BinaryOp>();
        Token op = previous_token();
        switch (op.type) {
            case TokenType::STAR: expr->op = "*"; break;
            case TokenType::SLASH: expr->op = "/"; break;
            case TokenType::PERCENT: expr->op = "%"; break;
            default: break;
        }
        expr->left = left;
        expr->right = parse_unary();
        left = expr;
    }
    
    return left;
}

ExprPtr Parser::parse_unary() {
    if (match({TokenType::MINUS, TokenType::NOT})) {
        auto expr = std::make_shared<UnaryOp>();
        expr->op = previous_token().type == TokenType::MINUS ? "-" : "not";
        expr->operand = parse_unary();
        return expr;
    }
    
    return parse_call();
}

ExprPtr Parser::parse_call() {
    auto expr = parse_primary();
    
    while (true) {
        if (match(TokenType::LPAREN)) {
            // function call
            auto call = std::make_shared<FunctionCall>();
            
            // get the function name from the expression
            if (auto id = std::dynamic_pointer_cast<Identifier>(expr)) {
                call->name = id->name;
            } else {
                throw error("Expected function name");
            }
            
            // paarse arguments
            if (!check(TokenType::RPAREN)) {
                do {
                    call->arguments.push_back(parse_expression());
                } while (match(TokenType::COMMA));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            expr = call;
            
        } else if (match(TokenType::DOT)) {
            // member access
            Token member = consume(TokenType::IDENTIFIER, "Expected member name");
            auto access = std::make_shared<MemberAccess>();
            access->object = expr;
            access->member = member.value;
            expr = access;
            
        } else if (match(TokenType::LBRACKET)) {
            // index access
            auto access = std::make_shared<IndexAccess>();
            access->object = expr;
            access->index = parse_expression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = access;
            
        } else {
            break;
        }
    }
    
    return expr;
}

ExprPtr Parser::parse_primary() {
    // literals
    if (match(TokenType::STRING)) {
        auto expr = std::make_shared<StringLiteral>();
        expr->value = previous_token().value;
        return expr;
    }
    
    if (match(TokenType::NUMBER)) {
        auto expr = std::make_shared<NumberLiteral>();
        std::string val = previous_token().value;
        expr->value = std::stod(val);
        expr->is_integer = (val.find('.') == std::string::npos);
        return expr;
    }
    
    if (match(TokenType::TRUE)) {
        auto expr = std::make_shared<BoolLiteral>();
        expr->value = true;
        return expr;
    }
    
    if (match(TokenType::FALSE)) {
        auto expr = std::make_shared<BoolLiteral>();
        expr->value = false;
        return expr;
    }
    
    if (match(TokenType::SYMBOL)) {
        auto expr = std::make_shared<Symbol>();
        expr->name = previous_token().value;
        return expr;
    }
    
    if (match(TokenType::IDENTIFIER)) {
        auto expr = std::make_shared<Identifier>();
        expr->name = previous_token().value;
        return expr;
    }
    
    // array literal
    if (match(TokenType::LBRACKET)) {
        return parse_array();
    }
    
    // hash literal
    if (match(TokenType::LBRACE)) {
        return parse_hash();
    }
    
    // grouped expression
    if (match(TokenType::LPAREN)) {
        auto expr = parse_expression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw error("Expected expression");
}

ExprPtr Parser::parse_array() {
    auto expr = std::make_shared<ArrayLiteral>();
    
    if (!check(TokenType::RBRACKET)) {
        do {
            skip_newlines();
            expr->elements.push_back(parse_expression());
            skip_newlines();
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RBRACKET, "Expected ']' after array elements");
    return expr;
}

ExprPtr Parser::parse_hash() {
    auto expr = std::make_shared<HashLiteral>();
    
    if (!check(TokenType::RBRACE)) {
        do {
            skip_newlines();
            auto key = parse_expression();
            consume(TokenType::COLON, "Expected ':' in hash literal");
            auto value = parse_expression();
            expr->pairs.push_back({key, value});
            skip_newlines();
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RBRACE, "Expected '}' after hash elements");
    return expr;
}

// token helpers

Token Parser::current_token() const {
    if (m_current >= m_tokens.size()) {
        return Token{TokenType::END_OF_FILE, "", 0, 0};
    }
    return m_tokens[m_current];
}

Token Parser::previous_token() const {
    if (m_current == 0) {
        return Token{TokenType::ERROR, "", 0, 0};
    }
    return m_tokens[m_current - 1];
}

Token Parser::advance() {
    if (!is_at_end()) {
        m_current++;
    }
    return previous_token();
}

bool Parser::check(TokenType type) const {
    return current_token().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    throw error(message);
}

void Parser::skip_newlines() {
    while (match(TokenType::NEWLINE)) {}
}

bool Parser::is_at_end() const {
    return current_token().type == TokenType::END_OF_FILE;
}

ParseError Parser::error(const std::string& message) {
    Token token = current_token();
    return ParseError(message + " at '" + token.value + "'", token.line, token.column);
}

} // namespace iris::lang
