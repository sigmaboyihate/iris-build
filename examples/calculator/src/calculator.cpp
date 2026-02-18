#include "calculator.hpp"

namespace calc {

Calculator::Calculator() = default;

double Calculator::calculate(const std::string& expression) {
    m_last_error.clear();
    
    try {
        Lexer lexer(expression);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto ast = parser.parse();
        
        return m_evaluator.evaluate(ast);
    }
    catch (const CalcError& e) {
        m_last_error = e.what();
        throw;
    }
}

void Calculator::set_variable(const std::string& name, double value) {
    m_evaluator.set_variable(name, value);
}

double Calculator::get_variable(const std::string& name) const {
    return m_evaluator.get_variable(name);
}

void Calculator::clear() {
    m_evaluator.clear_variables();
    m_last_error.clear();
}

} // namespace calc
