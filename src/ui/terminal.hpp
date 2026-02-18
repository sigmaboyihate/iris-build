#pragma once

#include <string>
#include <iostream>

namespace iris::ui {

enum class Color {
    Default = 0,
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    Gray = 90,
    BrightRed = 91,
    BrightGreen = 92,
    BrightYellow = 93,
    BrightBlue = 94,
    BrightMagenta = 95,
    BrightCyan = 96,
    BrightWhite = 97
};

enum class Style {
    Normal = 0,
    Bold = 1,
    Dim = 2,
    Italic = 3,
    Underline = 4
};

class Terminal {
public:
    static void init();
    static void reset();
    
    // Color and style
    static void set_color(Color fg);
    static void set_color(Color fg, Color bg);
    static void set_style(Style style);
    static void reset_style();
    
    // Formatted output
    static void print_styled(const std::string& text, Color color, 
                             Style style = Style::Normal);
    
    // Status messages
    static void info(const std::string& label, const std::string& message = "");
    static void success(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void hint(const std::string& message);
    static void debug(const std::string& message);
    
    // Headers and formatting
    static void header(const std::string& title);
    static void subheader(const std::string& title);
    static void separator(char c = '-', int width = 60);
    static void newline();
    
    // Input
    static std::string prompt(const std::string& message);
    static bool confirm(const std::string& message, bool default_yes = true);
    
    // Utility
    static int get_width();
    static bool is_tty();
    static bool supports_color();
    static void clear_line();
    static void move_up(int lines = 1);
    static void move_cursor(int row, int col);
    static void hide_cursor();
    static void show_cursor();

private:
    static bool s_color_enabled;
    static bool s_initialized;
    
    static std::string color_code(Color color);
    static std::string style_code(Style style);
};

// RAII style guard
class StyleGuard {
public:
    StyleGuard(Color color, Style style = Style::Normal);
    ~StyleGuard();
private:
    bool m_active;
};

} // namespace iris::ui