#include "calculator.hpp"

namespace calc {

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {}

const Token& Parser::current() const {
    if (m_pos >= m_tokens.size()) {
        return m_tokens.back();
    }
    return m_tokens[m_pos];
}

const Token& Parser::peek(int offset) const {
    size_t pos = m_pos + offset;
    if (pos >= m_tokens.size()) {
        return m_tokens.back();
    }
    return m_tokens[pos];
}

void Parser::advance() {
    if (m_pos < m_tokens.size()) m_pos++;
}

bool Parser::match(TokenType type) {
    if (current().type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TokenType type, const std::string& message) {
    if (!match(type)) {
        throw ParserError(message, current().position);
    }
}

ASTNode Parser::parse() {
    ASTNode result = parse_expression();
    if (current().type != TokenType::End) {
        throw ParserError("Unexpected token after expression", current().position);
    }
    return result;
}

ASTNode Parser::parse_expression() {
    return parse_assignment();
}

ASTNode Parser::parse_assignment() {
    if (current().type == TokenType::Identifier && peek().type == TokenType::Equals) {
        std::string name = current().value;
        advance(); // identifier
        advance(); // equals
        ASTNode value = parse_additive();
        return std::make_shared<AssignmentNode>(name, std::move(value));
    }
    return parse_additive();
}

ASTNode Parser::parse_additive() {
    ASTNode left = parse_multiplicative();
    
    while (current().type == TokenType::Plus || current().type == TokenType::Minus) {
        char op = current().value[0];
        advance();
        ASTNode right = parse_multiplicative();
        left = std::make_shared<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

ASTNode Parser::parse_multiplicative() {
    ASTNode left = parse_power();
    
    while (current().type == TokenType::Multiply || 
           current().type == TokenType::Divide ||
           current().type == TokenType::Modulo) {
        char op = current().value[0];
        advance();
        ASTNode right = parse_power();
        left = std::make_shared<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    
    return left;
}

ASTNode Parser::parse_power() {
    ASTNode left = parse_unary();
    
    if (current().type == TokenType::Power) {
        advance();
        ASTNode right = parse_power(); // Right associative
        left = std::make_shared<BinaryOpNode>('^', std::move(left), std::move(right));
    }
    
    return left;
}

ASTNode Parser::parse_unary() {
    if (current().type == TokenType::Minus) {
        advance();
        ASTNode operand = parse_unary();
        return std::make_shared<UnaryOpNode>('-', std::move(operand));
    }
    if (current().type == TokenType::Plus) {
        advance();
        return parse_unary();
    }
    return parse_primary();
}

ASTNode Parser::parse_primary() {
    const Token& tok = current();
    
    if (tok.type == TokenType::Number) {
        double value = std::stod(tok.value);
        advance();
        return std::make_shared<NumberNode>(value);
    }
    
    if (tok.type == TokenType::Identifier) {
        std::string name = tok.value;
        advance();
        
        if (current().type == TokenType::LeftParen) {
            return parse_function_call(name);
        }
        
        return std::make_shared<VariableNode>(name);
    }
    
    if (tok.type == TokenType::LeftParen) {
        advance();
        ASTNode expr = parse_expression();
        expect(TokenType::RightParen, "Expected ')'");
        return expr;
    }
    
    throw ParserError("Unexpected token: " + tok.value, tok.position);
}

ASTNode Parser::parse_function_call(const std::string& name) {
    expect(TokenType::LeftParen, "Expected '('");
    
    std::vector<ASTNode> args;
    
    if (current().type != TokenType::RightParen) {
        args.push_back(parse_expression());
        
        while (current().type == TokenType::Comma) {
            advance();
            args.push_back(parse_expression());
        }
    }
    
    expect(TokenType::RightParen, "Expected ')'");
    
    return std::make_shared<FunctionCallNode>(name, std::move(args));
}

} // namespace calc
