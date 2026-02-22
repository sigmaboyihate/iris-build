#include <iostream>
#include <string>
#include <vector>

#ifdef DEBUG
#define LOG(msg) std::cerr << "[DEBUG] " << msg << std::endl
#else
#define LOG(msg)
#endif
// THIS ASCII ART WAS HARD
void print_banner() {
    std::cout << R"(
  ╦╦═╗╦╔═╗  ╔╗ ╦ ╦╦╦  ╔╦╗
  ║╠╦╝║╚═╗  ╠╩╗║ ║║║   ║║
  ╩╩╚═╩╚═╝  ╚═╝╚═╝╩╩═╝═╩╝
    )" << std::endl;
}

int main(int argc, char* argv[]) {
    LOG("Starting application");

    print_banner();

    std::cout << "Hello from Iris Build System!" << std::endl;
    std::cout << std::endl;

    if (argc > 1) {
        std::cout << "Arguments received:" << std::endl;
        for (int i = 1; i < argc; i++) {
            std::cout << "  [" << i << "] " << argv[i] << std::endl;
        }
    }

    LOG("Application finished");

    return 0;
}
