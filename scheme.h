#pragma once

#include <string>

class Interpreter {
public:
    Interpreter();
    std::string Run(const std::string& input);
};
