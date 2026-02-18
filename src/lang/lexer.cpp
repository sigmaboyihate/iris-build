#include "lexer.hpp"
#include <stdexcept>
#include <cctype>

namespace iris::lang {

const std::map<std::string, TokenType> Lexer::keywords = {
    {"project", TokenType::PROJECT},
    {"executable", TokenType::EXECUTABLE},
    {"library", TokenType::LIBRARY},
    {"shared_library", TokenType::SHARED_LIBRARY},
    {"static_library", TokenType::STATIC_LIBRARY},
    {"compiler", TokenType::COMPILER},
    {"dependency", TokenType::DEPENDENCY},
    {"task", TokenType::TASK},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"unless", TokenType::UNLESS},
    {"for", TokenType::FOR},
    {"in", TokenType::IN},
    {"do", TokenType::DO},
    {"end", TokenType::END},
    {"fn", TokenType::FN},
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"nil", TokenType::NIL},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT}
};

std::string Token::type_name() const {
    switch (type) {
        case TokenType::STRING: return "STRING";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::SYMBOL: return "SYMBOL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::PROJECT: return "PROJECT";
        case TokenType::EXECUTABLE: return "EXECUTABLE";
        case TokenType::LIBRARY: return "LIBRARY";
        case TokenType::DO: return "DO";
        case TokenType::END: return "END";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "TOKEN";
    }
}

Lexer::Lexer(const std::string& source) : m_source(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!is_at_end()) {
        Token token = next_token();
        if (token.type != TokenType::ERROR) {
            tokens.push_back(token);
        }
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }
    
    return tokens;
}

Token Lexer::next_token() {
    skip_whitespace();
    
    if (is_at_end()) {
        return make_token(TokenType::END_OF_FILE);
    }
    
    char c = current();
    
    // Comments
    if (c == '#' && peek_char() != '{') {
        skip_comment();
        return next_token();
    }
    if (c == '/' && peek_char() == '/') {
        skip_comment();
        return next_token();
    }
    
    // Newlines (significant in some contexts)
    if (c == '\n') {
        advance();
        return make_token(TokenType::NEWLINE);
    }
    
    // Strings
    if (c == '"' || c == '\'') {
        return scan_string();
    }
    
    // Numbers
    if (is_digit(c)) {
        return scan_number();
    }
    
    // Symbols (:name)
    if (c == ':' && is_alpha(peek_char())) {
        return scan_symbol();
    }
    
    // Identifiers and keywords
    if (is_alpha(c) || c == '_') {
        return scan_identifier_or_keyword();
    }
    
    // Operators and delimiters
    advance();
    switch (c) {
        case '(': return make_token(TokenType::LPAREN);
        case ')': return make_token(TokenType::RPAREN);
        case '[': return make_token(TokenType::LBRACKET);
        case ']': return make_token(TokenType::RBRACKET);
        case '{': return make_token(TokenType::LBRACE);
        case '}': return make_token(TokenType::RBRACE);
        case ',': return make_token(TokenType::COMMA);
        case '.': return make_token(TokenType::DOT);
        case ';': return make_token(TokenType::SEMICOLON);
        case ':': return make_token(TokenType::COLON);
        case '+':
            if (match('=')) return make_token(TokenType::PLUSEQ);
            return make_token(TokenType::PLUS);
        case '-':
            if (match('>')) return make_token(TokenType::ARROW);
            if (match('=')) return make_token(TokenType::MINUSEQ);
            return make_token(TokenType::MINUS);
        case '*': return make_token(TokenType::STAR);
        case '/': return make_token(TokenType::SLASH);
        case '%': return make_token(TokenType::PERCENT);
        case '=':
            if (match('=')) return make_token(TokenType::EQEQ);
            if (match('>')) return make_token(TokenType::FAT_ARROW);
            return make_token(TokenType::EQ);
        case '!':
            if (match('=')) return make_token(TokenType::NEQ);
            return make_token(TokenType::NOT);
        case '<':
            if (match('=')) return make_token(TokenType::LTE);
            return make_token(TokenType::LT);
        case '>':
            if (match('=')) return make_token(TokenType::GTE);
            return make_token(TokenType::GT);
        case '#':
            if (match('{')) return make_token(TokenType::INTERPOLATION_START);
            skip_comment();
            return next_token();
    }
    
    return make_token(TokenType::ERROR, std::string(1, c));
}

Token Lexer::peek() {
    size_t saved_pos = m_pos;
    int saved_line = m_line;
    int saved_col = m_column;
    
    Token token = next_token();
    
    m_pos = saved_pos;
    m_line = saved_line;
    m_column = saved_col;
    
    return token;
}

bool Lexer::has_more() const {
    return !is_at_end();
}

char Lexer::current() const {
    if (is_at_end()) return '\0';
    return m_source[m_pos];
}

char Lexer::peek_char(int offset) const {
    if (m_pos + offset >= m_source.length()) return '\0';
    return m_source[m_pos + offset];
}

void Lexer::advance() {
    if (!is_at_end()) {
        if (m_source[m_pos] == '\n') {
            m_line++;
            m_column = 1;
        } else {
            m_column++;
        }
        m_pos++;
    }
}

void Lexer::skip_whitespace() {
    while (!is_at_end()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skip_comment() {
    while (!is_at_end() && current() != '\n') {
        advance();
    }
}

Token Lexer::make_token(TokenType type, const std::string& value) {
    return Token{type, value, m_line, m_column};
}

Token Lexer::scan_string() {
    char quote = current();
    advance();  // Skip opening quote
    
    std::string value;
    
    while (!is_at_end() && current() != quote) {
        if (current() == '\\') {
            advance();
            if (!is_at_end()) {
                switch (current()) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    case '\'': value += '\''; break;
                    default: value += current(); break;
                }
                advance();
            }
        } else {
            value += current();
            advance();
        }
    }
    
    if (is_at_end()) {
        return make_token(TokenType::ERROR, "Unterminated string");
    }
    
    advance();  // Skip closing quote
    return make_token(TokenType::STRING, value);
}

Token Lexer::scan_number() {
    std::string value;
    
    while (!is_at_end() && is_digit(current())) {
        value += current();
        advance();
    }
    
    if (current() == '.' && is_digit(peek_char())) {
        value += current();
        advance();
        
        while (!is_at_end() && is_digit(current())) {
            value += current();
            advance();
        }
    }
    
    return make_token(TokenType::NUMBER, value);
}

Token Lexer::scan_identifier_or_keyword() {
    std::string value;
    
    while (!is_at_end() && (is_alphanumeric(current()) || current() == '_')) {
        value += current();
        advance();
    }
    
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return make_token(it->second, value);
    }
    
    return make_token(TokenType::IDENTIFIER, value);
}

Token Lexer::scan_symbol() {
    advance();  // Skip the colon
    
    std::string value;
    while (!is_at_end() && (is_alphanumeric(current()) || current() == '_')) {
        value += current();
        advance();
    }
    
    return make_token(TokenType::SYMBOL, value);
}

bool Lexer::match(char expected) {
    if (is_at_end() || current() != expected) {
        return false;
    }
    advance();
    return true;
}

bool Lexer::is_at_end() const {
    return m_pos >= m_source.length();
}

bool Lexer::is_digit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::is_alpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::is_alphanumeric(char c) const {
    return is_alpha(c) || is_digit(c);
}

} // namespace iris::lang