#pragma once

#include <string>
#include <vector>
#include <variant>
#include <map>

namespace iris::lang {

enum class TokenType {
    // Literals
    STRING,
    NUMBER,
    SYMBOL,        // :name
    IDENTIFIER,
    
    // Keywords
    PROJECT,
    EXECUTABLE,
    LIBRARY,
    SHARED_LIBRARY,
    STATIC_LIBRARY,
    COMPILER,
    DEPENDENCY,
    TASK,
    IF,
    ELSE,
    UNLESS,
    FOR,
    IN,
    DO,
    END,
    FN,
    RETURN,
    TRUE,
    FALSE,
    NIL,
    AND,
    OR,
    NOT,
    
    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    EQ,
    EQEQ,
    NEQ,
    LT,
    GT,
    LTE,
    GTE,
    PLUSEQ,
    MINUSEQ,
    
    // Delimiters
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    COMMA,
    DOT,
    COLON,
    SEMICOLON,
    ARROW,         // ->
    FAT_ARROW,     // =>
    
    // Special
    INTERPOLATION_START,  // #{
    NEWLINE,
    INDENT,
    DEDENT,
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    std::string type_name() const;
};

class Lexer {
public:
    Lexer(const std::string& source);
    
    std::vector<Token> tokenize();
    Token next_token();
    Token peek();
    
    bool has_more() const;
    
private:
    std::string m_source;
    size_t m_pos = 0;
    int m_line = 1;
    int m_column = 1;
    
    char current() const;
    char peek_char(int offset = 1) const;
    void advance();
    void skip_whitespace();
    void skip_comment();
    
    Token make_token(TokenType type, const std::string& value = "");
    Token scan_string();
    Token scan_number();
    Token scan_identifier_or_keyword();
    Token scan_symbol();
    
    bool match(char expected);
    bool is_at_end() const;
    bool is_digit(char c) const;
    bool is_alpha(char c) const;
    bool is_alphanumeric(char c) const;
    
    static const std::map<std::string, TokenType> keywords;
};

} // namespace iris::lang