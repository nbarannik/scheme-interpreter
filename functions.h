#pragma once

#include <optional>
#include <vector>
#include <unordered_map>
#include <string>
#include "object.h"
#include "error.h"

// Helpers

std::vector<std::string> GetSymbolsList(ObjectPtr obj);

std::vector<ObjectPtr> GetArgList(ObjectPtr obj);

ObjectPtr GetHeadFromList(ObjectPtr obj);

ObjectPtr GetTailFromList(ObjectPtr obj);

ObjectPtr GetListTailFromKthElement(ObjectPtr obj, size_t k);

ObjectPtr GetListKthElement(ObjectPtr obj, size_t k);

ObjectPtr GetListFromArgs(const std::vector<ObjectPtr>& args);

ObjectPtr GetBoolean(bool value);

void EvaluateArgs(std::vector<ObjectPtr>& args_list);

template <typename Error>
void CheckArgumentsCount(const std::vector<ObjectPtr>& args_list, size_t min_count = 0,
                         size_t max_count = SIZE_MAX) {
    if (args_list.size() >= min_count && args_list.size() <= max_count) {
        return;
    }
    throw Error("Expected from " + std::to_string(min_count) + " to " + std::to_string(max_count) +
                " arguments, got " + std::to_string(args_list.size()));
}

bool IsCorrectList(ObjectPtr obj);

bool IsPair(ObjectPtr obj);

bool IsNull(ObjectPtr obj);

bool IsNumber(ObjectPtr obj);

bool IsSymbol(ObjectPtr obj);

bool IsBoolean(ObjectPtr obj);

bool IsFalse(ObjectPtr obj);

template <typename T>
bool IsAll(const std::vector<ObjectPtr>& args);

// Universal functions

class LambdaFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class IfFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class QuoteFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class IsFunction : public Function {
public:
    template <class F>
    explicit IsFunction(F&& f) : predicate_(f) {
    }

    ObjectPtr Apply(ObjectPtr obj) override;

private:
    bool (*predicate_)(ObjectPtr obj);
};

class DefineFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class SetFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

// List functions

class ConsFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class CarFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class CdrFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class ListFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class ListTailFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class ListRefFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class SetCarFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

class SetCdrFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

// Number functions

template <typename Comparator>
class CompareFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override {
        std::vector<ObjectPtr> args = GetArgList(obj);
        EvaluateArgs(args);
        if (!IsAll<Number>(args)) {
            throw RuntimeError("Arguments of compare function should be numbers");
        }
        Comparator cmp;
        bool res = true;
        for (size_t i = 0; !args.empty() && i < args.size() - 1; ++i) {
            res &= cmp(As<Number>(args[i])->GetValue(), As<Number>(args[i + 1])->GetValue());
        }
        return (res ? std::make_shared<Symbol>("#t") : std::make_shared<Symbol>("#f"));
    }
};

template <typename T>
struct Min {
    T operator()(T t1, T t2) {
        return std::min(t1, t2);
    }
};

template <typename T>
struct Max {
    T operator()(T t1, T t2) {
        return std::max(t1, t2);
    }
};

template <typename F>
class ArithmeticFunction : public Function {
public:
    ArithmeticFunction() = default;
    explicit ArithmeticFunction(int64_t base_value) : base_value_(base_value) {
    }

    ObjectPtr Apply(ObjectPtr obj) override {
        std::vector<ObjectPtr> args = GetArgList(obj);
        EvaluateArgs(args);
        if (!IsAll<Number>(args)) {
            throw RuntimeError("Arguments of arithmetic function should be numbers");
        }
        if (args.empty()) {
            if (base_value_) {
                return std::make_shared<Number>(base_value_.value());
            }
            throw RuntimeError("Function expected at least 1 argument, got 0");
        }
        F func;
        int64_t res = As<Number>(args[0])->GetValue();
        for (size_t i = 1; i < args.size(); ++i) {
            res = func(res, As<Number>(args[i])->GetValue());
        }
        return std::make_shared<Number>(res);
    }

private:
    std::optional<int64_t> base_value_;
};

class AbsFunction : public Function {
public:
    ObjectPtr Apply(ObjectPtr obj) override;
};

// boolean functions

template <typename F>
class BooleanFunction : public Function {
public:
    BooleanFunction(bool base_value, bool return_value)
        : base_value_(base_value), return_value_(return_value) {
    }

    ObjectPtr Apply(ObjectPtr obj) override {
        std::vector<ObjectPtr> args = GetArgList(obj);
        if (args.empty()) {
            return GetBoolean(base_value_);
        }
        F func;
        bool res = base_value_;
        for (size_t i = 0; i < args.size(); ++i) {
            args[i] = args[i]->Evaluate();
            res = func(res, !IsFalse(args[i]));
            if (res == return_value_) {
                return args[i];
            }
        }
        return args.back();
    }

private:
    bool base_value_, return_value_;
};
