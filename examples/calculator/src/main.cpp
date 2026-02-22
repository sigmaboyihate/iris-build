#include "calculator.hpp"
#include <iostream>
#include <string>
#include <iomanip>

void print_banner() {
    std::cout << R"(
  ╔═══════════════════════════════════════════════════════╗
  ║           IRIS CALCULATOR - Built with Iris           ║
  ╠═══════════════════════════════════════════════════════╣
  ║  Enter expressions to evaluate. Type 'help' for help. ║
  ║  Type 'quit' or 'exit' to exit.                       ║
  ╚═══════════════════════════════════════════════════════╝
)" << std::endl;
}

void print_help() {
    std::cout << R"(
Available operations:
  +, -, *, /     Basic arithmetic
  ^, **          Exponentiation
  %              Modulo
  ()             Grouping

Available functions:
  abs(x)         Absolute value
  sqrt(x)        Square root
  pow(x, y)      x to the power of y
  exp(x)         e^x
  log(x)         Natural logarithm
  log10(x)       Base-10 logarithm
  log2(x)        Base-2 logarithm
  
  sin(x), cos(x), tan(x)     Trigonometric functions
  asin(x), acos(x), atan(x)  Inverse trigonometric
  sinh(x), cosh(x), tanh(x)  Hyperbolic functions
  
  floor(x), ceil(x), round(x), trunc(x)  Rounding
  min(a, b, ...), max(a, b, ...)         Min/Max
  
  pi(), e()      Mathematical constants

Variables:
  pi, e, tau, phi   Built-in constants
  x = 5             Assign value to variable
  x + 2             Use variable in expression

Examples:
  2 + 3 * 4
  sqrt(16) + pow(2, 3)
  sin(pi / 2)
  x = 10
  x * 2 + 5
)" << std::endl;
}

void print_variables(const calc::Calculator& calc) {
    std::cout << "\nDefined variables:" << std::endl;
    // note: this would need accessor to internal evaluator
    std::cout << "  pi = 3.14159..." << std::endl;
    std::cout << "  e = 2.71828..." << std::endl;
    std::cout << "  tau = 6.28318..." << std::endl;
    std::cout << "  phi = 1.61803..." << std::endl;
}

int main(int argc, char* argv[]) {
    calc::Calculator calculator;
    
    // if arguments given uh it works
    if (argc > 1) {
        std::string expression;
        for (int i = 1; i < argc; i++) {
            if (i > 1) expression += " ";
            expression += argv[i];
        }
        
        try {
            double result = calculator.calculate(expression);
            std::cout << std::setprecision(15) << result << std::endl;
            return 0;
        }
        catch (const calc::CalcError& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    
    // interactive mode
    print_banner();
    
    std::string line;
    std::cout << ">>> ";
    
    while (std::getline(std::cin, line)) {
        // trim whitespace
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        if (start == std::string::npos) {
            std::cout << ">>> ";
            continue;
        }
        line = line.substr(start, end - start + 1);
        
        // hanlde commands
        if (line == "quit" || line == "exit" || line == "q") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        if (line == "help" || line == "?") {
            print_help();
            std::cout << ">>> ";
            continue;
        }
        
        if (line == "clear") {
            calculator.clear();
            std::cout << "Variables cleared." << std::endl;
            std::cout << ">>> ";
            continue;
        }
        
        if (line == "vars" || line == "variables") {
            print_variables(calculator);
            std::cout << ">>> ";
            continue;
        }
        
        // evvaluate expressions
        try {
            double result = calculator.calculate(line);
            std::cout << "= " << std::setprecision(15) << result << std::endl;
        }
        catch (const calc::CalcError& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        std::cout << ">>> ";
    }
    
    return 0;
}
