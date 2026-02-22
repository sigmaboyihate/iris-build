#include "cli/cli.hpp"
#include "ui/terminal.hpp"

int main(int argc, char* argv[]) {
    iris::ui::Terminal::init();

    iris::cli::CLI app("iris", "A modern build system with expressive syntax");
    return app.run(argc, argv);
}
