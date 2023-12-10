#include "functions.h"
#include "scope.h"
#include <utility>

// Object functions

ObjectPtr LambdaFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<SyntaxError>(args, 2);
    if (!IsCorrectList(args[0])) {
        throw SyntaxError("Wrong lambda syntax");
    }
    return std::make_shared<Lambda>(As<Cell>(obj)->GetSecond(), GetSymbolsList(args[0]),
                                    GetCurrentScope());
}

ObjectPtr IfFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<SyntaxError>(args, 2, 3);
    args[0] = args[0]->Evaluate();
    if (!IsFalse(args[0])) {
        return args[1]->Evaluate();
    } else if (args.size() == 2) {
        return nullptr;
    } else {
        return args[2]->Evaluate();
    }
}

ObjectPtr SetFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<SyntaxError>(args, 2, 2);
    if (!IsSymbol(args[0])) {
        throw RuntimeError("Name of variable should be Symbol");
    }
    args[1] = args[1]->Evaluate();
    GetCurrentScope()->Set(As<Symbol>(args[0])->GetName(), args[1]);
    return nullptr;
}

ObjectPtr DefineFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<SyntaxError>(args, 2);
    if (IsSymbol(args[0])) {
        if (args.size() > 2) {
            throw SyntaxError("Define expects 2 arguments, got " + std::to_string(args.size()));
        }
        args[1] = args[1]->Evaluate();
        GetCurrentScope()->Define(As<Symbol>(args[0])->GetName(), args[1]);
    } else if (IsCorrectList(args[0])) {
        if (auto name_ptr = GetHeadFromList(args[0]); Is<Symbol>(name_ptr)) {
            std::string lambda_name = As<Symbol>(name_ptr)->GetName();
            ObjectPtr init_list = GetTailFromList(args[0]);
            ObjectPtr lambda = std::make_shared<Lambda>(
                GetTailFromList(obj), GetSymbolsList(init_list), GetCurrentScope());
            GetCurrentScope()->Define(lambda_name, lambda);
        } else {
            throw RuntimeError("Name of Lambda should be Symbol");
        }
    } else {
        throw RuntimeError("First argument of define should be Symbol or Lambda declaration");
    }
    return nullptr;
}

ObjectPtr QuoteFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 1, 1);
    return args[0];
}

ObjectPtr IsFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 1, 1);
    EvaluateArgs(args);
    return GetBoolean(predicate_(args[0]));
}

ObjectPtr ConsFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 2, 2);
    EvaluateArgs(args);
    ObjectPtr pair = std::make_shared<Cell>();
    As<Cell>(pair)->GetFirst() = args[0];
    As<Cell>(pair)->GetSecond() = args[1];
    return pair;
}

ObjectPtr CarFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 1, 1);
    EvaluateArgs(args);
    return GetHeadFromList(args[0]);
}

ObjectPtr SetCarFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<SyntaxError>(args, 2, 2);
    EvaluateArgs(args);
    if (!Is<Cell>(args[0])) {
        throw RuntimeError("set-car! first argument is not a list");
    }
    As<Cell>(args[0])->GetFirst() = args[1];
    return nullptr;
}

ObjectPtr SetCdrFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<SyntaxError>(args, 2, 2);
    EvaluateArgs(args);
    if (!Is<Cell>(args[0])) {
        throw RuntimeError("set-cdr! first argument is not a list");
    }
    As<Cell>(args[0])->GetSecond() = args[1];
    return nullptr;
}

ObjectPtr CdrFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 1, 1);
    EvaluateArgs(args);
    return GetTailFromList(args[0]);
}

ObjectPtr ListFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    EvaluateArgs(args);
    return GetListFromArgs(args);
}

ObjectPtr ListTailFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 2, 2);
    EvaluateArgs(args);
    if (!Is<Number>(args[1])) {
        throw RuntimeError("Second argument should be Number");
    }
    return GetListTailFromKthElement(args[0], As<Number>(args[1])->GetValue());
}

ObjectPtr ListRefFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 2, 2);
    EvaluateArgs(args);
    if (!Is<Number>(args[1])) {
        throw RuntimeError("Second argument should be Number");
    }
    return GetListKthElement(args[0], As<Number>(args[1])->GetValue());
}

ObjectPtr AbsFunction::Apply(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    CheckArgumentsCount<RuntimeError>(args, 1, 1);
    EvaluateArgs(args);
    if (!IsAll<Number>(args)) {
        throw RuntimeError("Argument of abs function should be number");
    }
    return std::make_shared<Number>(As<Number>(args[0])->GetValue() >= 0
                                        ? As<Number>(args[0])->GetValue()
                                        : -As<Number>(args[0])->GetValue());
}

// Helpers

std::vector<std::string> GetSymbolsList(ObjectPtr obj) {
    std::vector<ObjectPtr> args = GetArgList(obj);
    std::vector<std::string> vars;
    for (auto a : args) {
        if (!Is<Symbol>(a)) {
            throw SyntaxError("Lambda parameters should be Symbols");
        }
        vars.push_back(As<Symbol>(a)->GetName());
    }
    return vars;
}

std::vector<ObjectPtr> GetArgList(ObjectPtr obj) {
    std::vector<ObjectPtr> list;
    while (obj) {
        if (!Is<Cell>(obj)) {
            throw SyntaxError("");
        }
        list.push_back(As<Cell>(obj)->GetFirst());
        obj = As<Cell>(obj)->GetSecond();
    }
    return list;
}

ObjectPtr GetHeadFromList(ObjectPtr obj) {
    if (!obj || !Is<Cell>(obj)) {
        throw RuntimeError("");
    }
    return As<Cell>(obj)->GetFirst();
}

ObjectPtr GetTailFromList(ObjectPtr obj) {
    if (!obj || !Is<Cell>(obj)) {
        throw RuntimeError("");
    }
    return As<Cell>(obj)->GetSecond();
}

ObjectPtr GetListTailFromKthElement(ObjectPtr obj, size_t k) {
    for (size_t i = 0; i < k; ++i) {
        if (!obj || !Is<Cell>(obj)) {
            throw RuntimeError("");
        }
        obj = As<Cell>(obj)->GetSecond();
    }
    return obj;
}

ObjectPtr GetListKthElement(ObjectPtr obj, size_t k) {
    ObjectPtr tail = GetListTailFromKthElement(obj, k);
    if (!tail) {
        throw RuntimeError("Index error");
    }
    return GetHeadFromList(tail);
}

ObjectPtr GetBoolean(bool value) {
    return (value ? std::make_shared<Symbol>("#t") : std::make_shared<Symbol>("#f"));
}

ObjectPtr GetListFromArgs(const std::vector<ObjectPtr>& args) {
    if (args.empty()) {
        return nullptr;
    }
    ObjectPtr root = std::make_shared<Cell>();
    ObjectPtr v = root;
    for (size_t i = 0; i < args.size(); ++i) {
        As<Cell>(v)->GetFirst() = args[i];
        if (i != args.size() - 1) {
            As<Cell>(v)->GetSecond() = std::make_shared<Cell>();
            v = As<Cell>(v)->GetSecond();
        }
    }
    return root;
}

void EvaluateArgs(std::vector<ObjectPtr>& args_list) {
    std::vector<ObjectPtr> evaluated_list;
    for (auto& a : args_list) {
        if (!a) {
            throw RuntimeError("Empty is not evaluatable");
        }
        a = a->Evaluate();
    }
}

bool IsCorrectList(ObjectPtr obj) {
    while (obj && Is<Cell>(obj)) {
        obj = As<Cell>(obj)->GetSecond();
    }
    return (obj.get() == nullptr);
}

bool IsPair(ObjectPtr obj) {
    return Is<Cell>(obj);
}

bool IsNull(ObjectPtr obj) {
    return obj.get() == nullptr;
}

bool IsNumber(ObjectPtr obj) {
    return (!IsNull(obj) && Is<Number>(obj));
}

bool IsSymbol(ObjectPtr obj) {
    return (!IsNull(obj) && Is<Symbol>(obj));
}

bool IsBoolean(ObjectPtr obj) {
    return (!IsNull(obj) && Is<Symbol>(obj) &&
            (As<Symbol>(obj)->GetName() == "#t" || As<Symbol>(obj)->GetName() == "#f"));
}

template <typename T>
bool IsAll(const std::vector<ObjectPtr>& args) {
    for (auto i : args) {
        if (!Is<T>(i)) {
            return false;
        }
    }
    return true;
}

bool IsFalse(ObjectPtr obj) {
    return (IsBoolean(obj) && As<Symbol>(obj)->GetName() == "#f");
}
