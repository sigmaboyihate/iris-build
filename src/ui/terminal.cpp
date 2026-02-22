#include "terminal.hpp"

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace iris::ui {

bool Terminal::s_color_enabled = false;
bool Terminal::s_initialized = false;

void Terminal::init() {
    if (s_initialized) return;
    s_initialized = true;

#ifdef _WIN32
    // enable ansi escape codes on widnows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif

    s_color_enabled = supports_color();
}

void Terminal::reset() {
    if (s_color_enabled) {
        std::cout << "\033[0m";
    }
}

void Terminal::set_color(Color fg) {
    if (s_color_enabled) {
        std::cout << "\033[" << static_cast<int>(fg) << "m";
    }
}

void Terminal::set_color(Color fg, Color bg) {
    if (s_color_enabled) {
        std::cout << "\033[" << static_cast<int>(fg) << ";" 
                  << (static_cast<int>(bg) + 10) << "m";
    }
}

void Terminal::set_style(Style style) {
    if (s_color_enabled) {
        std::cout << "\033[" << static_cast<int>(style) << "m";
    }
}

void Terminal::reset_style() {
    reset();
}

void Terminal::print_styled(const std::string& text, Color color, Style style) {
    if (s_color_enabled) {
        std::cout << "\033[" << static_cast<int>(style) << ";" 
                  << static_cast<int>(color) << "m"
                  << text << "\033[0m";
    } else {
        std::cout << text;
    }
}

void Terminal::info(const std::string& label, const std::string& message) {
    std::cout << "  ";
    print_styled("•", Color::Cyan, Style::Bold);
    std::cout << " ";
    print_styled(label, Color::White, Style::Bold);
    if (!message.empty()) {
        std::cout << ": ";
        print_styled(message, Color::Gray);
    }
    std::cout << "\n";
}
// took me a while to get a check mark
void Terminal::success(const std::string& message) {
    std::cout << "  ";
    print_styled("✓", Color::Green, Style::Bold);
    std::cout << " ";
    print_styled(message, Color::Green);
    std::cout << "\n";
}

void Terminal::warning(const std::string& message) {
    std::cout << "  ";
    print_styled("!", Color::Yellow, Style::Bold);
    std::cout << " ";
    print_styled("Warning: ", Color::Yellow, Style::Bold);
    std::cout << message << "\n";
}

void Terminal::error(const std::string& message) {
    std::cout << "  ";
    print_styled("(X)", Color::Red, Style::Bold);
    std::cout << " ";
    print_styled("Error: ", Color::Red, Style::Bold);
    std::cout << message << "\n";
}

void Terminal::hint(const std::string& message) {
    std::cout << "  ";
    print_styled("->", Color::Blue);
    std::cout << " ";
    print_styled(message, Color::Gray);
    std::cout << "\n";
}

void Terminal::debug(const std::string& message) {
#ifdef DEBUG
    std::cout << "  ";
    print_styled("[DEBUG]", Color::Magenta);
    std::cout << " " << message << "\n";
#endif
}

void Terminal::header(const std::string& title) {
    std::cout << "\n";
    print_styled("══", Color::Cyan);
    std::cout << " ";
    print_styled(title, Color::White, Style::Bold);
    std::cout << " ";
    
    int remaining = get_width() - static_cast<int>(title.length()) - 6;
    if (remaining > 0) {
        for (int i = 0; i < remaining; i++) {
            print_styled("═", Color::Cyan);
        }
    }
    std::cout << "\n\n";
}

void Terminal::subheader(const std::string& title) {
    std::cout << "\n";
    print_styled("──", Color::Gray);
    std::cout << " ";
    print_styled(title, Color::White);
    std::cout << " ";
    
    int remaining = get_width() - static_cast<int>(title.length()) - 6;
    if (remaining > 0) {
        for (int i = 0; i < remaining; i++) {
            print_styled("─", Color::Gray);
        }
    }
    std::cout << "\n";
}

void Terminal::separator(char c, int width) {
    std::cout << "  ";
    print_styled(std::string(width, c), Color::Gray);
    std::cout << "\n";
}

void Terminal::newline() {
    std::cout << "\n";
}

std::string Terminal::prompt(const std::string& message) {
    print_styled("? ", Color::Cyan, Style::Bold);
    std::cout << message << " ";
    
    std::string input;
    std::getline(std::cin, input);
    return input;
}

bool Terminal::confirm(const std::string& message, bool default_yes) {
    print_styled("? ", Color::Cyan, Style::Bold);
    std::cout << message;
    
    if (default_yes) {
        print_styled(" [Y/n] ", Color::Gray);
    } else {
        print_styled(" [y/N] ", Color::Gray);
    }
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.empty()) return default_yes;
    
    char c = std::tolower(input[0]);
    return c == 'y';
}

int Terminal::get_width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80;
#endif
}

bool Terminal::is_tty() {
    return isatty(fileno(stdout)) != 0;
}

bool Terminal::supports_color() {
    if (!is_tty()) return false;
    
    const char* term = std::getenv("TERM");
    if (!term) return false;
    
    std::string term_str(term);
    
    // am check for color terminal :D
    if (term_str.find("color") != std::string::npos ||
        term_str.find("xterm") != std::string::npos ||
        term_str.find("screen") != std::string::npos ||
        term_str.find("vt100") != std::string::npos ||
        term_str.find("linux") != std::string::npos ||
        term_str.find("ansi") != std::string::npos) {
        return true;
    }
    
    // check colorterm environment variable
    const char* colorterm = std::getenv("COLORTERM");
    if (colorterm) return true;
    
    // check NO_COLOR
    const char* no_color = std::getenv("NO_COLOR");
    if (no_color) return false;
    
    return true;
}

void Terminal::clear_line() {
    if (s_color_enabled) {
        std::cout << "\033[2K\r";
    }
}

void Terminal::move_up(int lines) {
    if (s_color_enabled && lines > 0) {
        std::cout << "\033[" << lines << "A";
    }
}

void Terminal::move_cursor(int row, int col) {
    if (s_color_enabled) {
        std::cout << "\033[" << row << ";" << col << "H";
    }
}

void Terminal::hide_cursor() {
    if (s_color_enabled) {
        std::cout << "\033[?25l";
    }
}

void Terminal::show_cursor() {
    if (s_color_enabled) {
        std::cout << "\033[?25h";
    }
}

std::string Terminal::color_code(Color color) {
    return "\033[" + std::to_string(static_cast<int>(color)) + "m";
}

std::string Terminal::style_code(Style style) {
    return "\033[" + std::to_string(static_cast<int>(style)) + "m";
}

// StyleGuard implementation

StyleGuard::StyleGuard(Color color, Style style) : m_active(Terminal::supports_color()) {
    if (m_active) {
        Terminal::set_style(style);
        Terminal::set_color(color);
    }
}

StyleGuard::~StyleGuard() {
    if (m_active) {
        Terminal::reset_style();
    }
}

} // namespace iris::ui
