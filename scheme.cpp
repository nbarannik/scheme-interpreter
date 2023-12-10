#include <sstream>
#include "scheme.h"
#include "tokenizer.h"
#include "parser.h"
#include "error.h"
#include "scope.h"

Interpreter::Interpreter() {
    SetCurrentScope(std::make_shared<Scope>());
    GetCurrentScope()->InitGlobalScope();
}

std::string Interpreter::Run(const std::string &input) {
    std::stringstream ss{input};
    Tokenizer tokenizer{&ss};
    auto syntax_tree = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("");
    }
    if (!syntax_tree) {
        throw RuntimeError("Lists are not evaluating");
    }

    auto res = syntax_tree->Evaluate();
    if (!res) {
        return "()";
    }
    return res->ToString();
}
