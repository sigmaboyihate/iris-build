#include "calculator.hpp"
#include <cctype>

namespace calc {

Lexer::Lexer(const std::string& input) : m_input(input) {}

char Lexer::current() const {
    if (m_pos >= m_input.length()) return '\0';
    return m_input[m_pos];
}

char Lexer::peek(int offset) const {
    size_t pos = m_pos + offset;
    if (pos >= m_input.length()) return '\0';
    return m_input[pos];
}

void Lexer::advance() {
    if (m_pos < m_input.length()) m_pos++;
}

void Lexer::skip_whitespace() {
    while (std::isspace(current())) {
        advance();
    }
}

Token Lexer::read_number() {
    int start = static_cast<int>(m_pos);
    std::string value;
    
    // Integer part
    while (std::isdigit(current())) {
        value += current();
        advance();
    }
    
    // Decimal part
    if (current() == '.' && std::isdigit(peek())) {
        value += current();
        advance();
        while (std::isdigit(current())) {
            value += current();
            advance();
        }
    }
    
    // Scientific notation
    if (current() == 'e' || current() == 'E') {
        value += current();
        advance();
        if (current() == '+' || current() == '-') {
            value += current();
            advance();
        }
        if (!std::isdigit(current())) {
            throw LexerError("Invalid scientific notation", start);
        }
        while (std::isdigit(current())) {
            value += current();
            advance();
        }
    }
    
    return Token(TokenType::Number, value, start);
}

Token Lexer::read_identifier() {
    int start = static_cast<int>(m_pos);
    std::string value;
    
    while (std::isalnum(current()) || current() == '_') {
        value += current();
        advance();
    }
    
    return Token(TokenType::Identifier, value, start);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (m_pos < m_input.length()) {
        skip_whitespace();
        if (m_pos >= m_input.length()) break;
        
        int pos = static_cast<int>(m_pos);
        char c = current();
        
        if (std::isdigit(c) || (c == '.' && std::isdigit(peek()))) {
            tokens.push_back(read_number());
        } else if (std::isalpha(c) || c == '_') {
            tokens.push_back(read_identifier());
        } else {
            switch (c) {
                case '+':
                    tokens.emplace_back(TokenType::Plus, "+", pos);
                    advance();
                    break;
                case '-':
                    tokens.emplace_back(TokenType::Minus, "-", pos);
                    advance();
                    break;
                case '*':
                    if (peek() == '*') {
                        tokens.emplace_back(TokenType::Power, "**", pos);
                        advance();
                        advance();
                    } else {
                        tokens.emplace_back(TokenType::Multiply, "*", pos);
                        advance();
                    }
                    break;
                case '/':
                    tokens.emplace_back(TokenType::Divide, "/", pos);
                    advance();
                    break;
                case '^':
                    tokens.emplace_back(TokenType::Power, "^", pos);
                    advance();
                    break;
                case '%':
                    tokens.emplace_back(TokenType::Modulo, "%", pos);
                    advance();
                    break;
                case '(':
                    tokens.emplace_back(TokenType::LeftParen, "(", pos);
                    advance();
                    break;
                case ')':
                    tokens.emplace_back(TokenType::RightParen, ")", pos);
                    advance();
                    break;
                case '=':
                    tokens.emplace_back(TokenType::Equals, "=", pos);
                    advance();
                    break;
                case ',':
                    tokens.emplace_back(TokenType::Comma, ",", pos);
                    advance();
                    break;
                default:
                    throw LexerError(std::string("Unexpected character: ") + c, pos);
            }
        }
    }
    
    tokens.emplace_back(TokenType::End, "", static_cast<int>(m_pos));
    return tokens;
}

} // namespace calc
