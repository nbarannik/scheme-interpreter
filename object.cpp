#include "object.h"
#include "error.h"
#include "functions.h"
#include "scope.h"
#include <string>

// Abstract Object

ObjectPtr Object::Apply(ObjectPtr) {
    throw RuntimeError("Object is not a function");
}

ObjectPtr Lambda::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    if (args.size() != initialize_list_.size()) {
        throw RuntimeError("Expected " + std::to_string(initialize_list_.size()) +
                           " arguments in lambda, got " + std::to_string(args.size()));
    }
    EvaluateArgs(args);

    std::shared_ptr<Scope> prev_scope = GetCurrentScope();
    SetCurrentScope(std::make_shared<Scope>());
    GetCurrentScope()->SetPreviousScope(scope_);

    for (size_t i = 0; i < initialize_list_.size(); ++i) {
        GetCurrentScope()->Define(initialize_list_[i], args[i]);
    }
    std::vector<ObjectPtr> evaluate_list = GetArgList(body_);
    ObjectPtr res;

    std::shared_ptr<Scope> test = GetCurrentScope();

    for (auto e : evaluate_list) {
        res = e->Evaluate();
    }

    SetCurrentScope(prev_scope);

    return res;
}

// Number

int64_t Number::GetValue() const {
    return value_;
}

std::string Number::ToString() const {
    return std::to_string(value_);
}

ObjectPtr Number::Evaluate() {
    return std::make_shared<Number>(value_);
}

// Symbol

const std::string& Symbol::GetName() const {
    return name_;
}

std::string Symbol::ToString() const {
    return name_;
}

ObjectPtr Symbol::Evaluate() {
    if (name_ == "#t" || name_ == "#f") {
        return std::make_shared<Symbol>(name_);
    }
    return GetCurrentScope()->Get(name_);
}

// Function

std::string Function::ToString() const {
    return "Function";
}

ObjectPtr Function::Evaluate() {
    throw RuntimeError("Functions are not evaluable");
}

// Cell

ObjectPtr Cell::GetFirst() const {
    return first_;
}

ObjectPtr Cell::GetSecond() const {
    return second_;
}

ObjectPtr& Cell::GetFirst() {
    return first_;
}

ObjectPtr& Cell::GetSecond() {
    return second_;
}

std::string Cell::ToStringInner() const {
    std::string res;
    res += GetFirst()->ToString();
    if (!GetSecond()) {
        return res;
    } else if (Is<Cell>(GetSecond())) {
        res += " " + As<Cell>(GetSecond())->ToStringInner();
    } else {
        res += " . " + GetSecond()->ToString();
    }
    return res;
}

std::string Cell::ToString() const {
    if (!GetFirst()) {
        size_t cnt = 2;
        ObjectPtr cell = GetSecond();
        while (cell) {
            if (!Is<Cell>(As<Cell>(cell)->GetSecond())) {
                throw RuntimeError("");
            } else {
                ++cnt;
                cell = As<Cell>(cell)->GetSecond();
            }
        }
        std::string res;
        for (size_t i = 0; i < cnt; ++i) {
            res += "(";
        }
        for (size_t i = 0; i < cnt; ++i) {
            res += ")";
        }
        return res;
    }
    std::string res = "(" + GetFirst()->ToString();
    if (!GetSecond()) {
        res += ")";
        return res;
    } else if (Is<Cell>(GetSecond())) {
        res += " " + As<Cell>(GetSecond())->ToStringInner();
    } else {
        res += " . " + GetSecond()->ToString();
    }
    res += ")";
    return res;
}

ObjectPtr Cell::Evaluate() {
    if (!GetFirst()) {
        throw RuntimeError("Lists are not self evaluating");
    }
    ObjectPtr function = GetFirst()->Evaluate();
    if (!function) {
        throw RuntimeError("Object is not a function");
    }
    return function->Apply(GetSecond());
}
