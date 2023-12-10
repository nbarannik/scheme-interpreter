#include "scope.h"
#include "functions.h"

static std::shared_ptr<Scope> current_scope;

std::shared_ptr<Scope> GetCurrentScope() {
    return current_scope;
}

void SetCurrentScope(std::shared_ptr<Scope> other) {
    if (!other) {
        RuntimeError("Scope error");
    }
    current_scope = other;
}

void Scope::SetPreviousScope(std::shared_ptr<Scope> other) {
    previous_scope_ = other;
}

void Scope::InitGlobalScope() {
    registered_functions_ = {
        // list
        {"quote", std::make_shared<QuoteFunction>()},
        {"pair?", std::make_shared<IsFunction>(IsPair)},
        {"null?", std::make_shared<IsFunction>(IsNull)},
        {"list?", std::make_shared<IsFunction>(IsCorrectList)},
        {"cons", std::make_shared<ConsFunction>()},
        {"car", std::make_shared<CarFunction>()},
        {"cdr", std::make_shared<CdrFunction>()},
        {"list", std::make_shared<ListFunction>()},
        {"list-tail", std::make_shared<ListTailFunction>()},
        {"list-ref", std::make_shared<ListRefFunction>()},
        {"set-car!", std::make_shared<SetCarFunction>()},
        {"set-cdr!", std::make_shared<SetCdrFunction>()},
        // integers
        {"number?", std::make_shared<IsFunction>(IsNumber)},
        {"<", std::make_shared<CompareFunction<std::less<int64_t>>>()},
        {">", std::make_shared<CompareFunction<std::greater<int64_t>>>()},
        {"<=", std::make_shared<CompareFunction<std::less_equal<int64_t>>>()},
        {">=", std::make_shared<CompareFunction<std::greater_equal<int64_t>>>()},
        {"=", std::make_shared<CompareFunction<std::equal_to<int64_t>>>()},
        {"+", std::make_shared<ArithmeticFunction<std::plus<int64_t>>>(0)},
        {"-", std::make_shared<ArithmeticFunction<std::minus<int64_t>>>()},
        {"*", std::make_shared<ArithmeticFunction<std::multiplies<int64_t>>>(1)},
        {"/", std::make_shared<ArithmeticFunction<std::divides<int64_t>>>()},
        {"min", std::make_shared<ArithmeticFunction<Min<int64_t>>>()},
        {"max", std::make_shared<ArithmeticFunction<Max<int64_t>>>()},
        {"abs", std::make_shared<AbsFunction>()},
        // booleans
        {"boolean?", std::make_shared<IsFunction>(IsBoolean)},
        {"not", std::make_shared<IsFunction>(IsFalse)},
        {"and", std::make_shared<BooleanFunction<std::logical_and<bool>>>(true, false)},
        {"or", std::make_shared<BooleanFunction<std::logical_or<bool>>>(false, true)},
        // symbols
        {"symbol?", std::make_shared<IsFunction>(IsSymbol)},
        {"define", std::make_shared<DefineFunction>()},
        {"set!", std::make_shared<SetFunction>()},
        // objects
        {"if", std::make_shared<IfFunction>()},
        {"lambda", std::make_shared<LambdaFunction>()},
    };
}

std::shared_ptr<Scope> Scope::GetPreviousScope() {
    return previous_scope_;
}

ObjectPtr Scope::Get(const std::string& s) {
    if (auto it = registered_functions_.find(s); it != registered_functions_.end()) {
        return (*it).second;
    } else if (previous_scope_) {
        return previous_scope_->Get(s);
    } else {
        throw NameError("Unknown identifier: " + s);
    }
}

void Scope::Define(const std::string& s, ObjectPtr object) {
    registered_functions_[s] = object;
}

void Scope::Set(const std::string& s, ObjectPtr object) {
    if (auto it = registered_functions_.find(s); it != registered_functions_.end()) {
        (*it).second = object;
    } else if (previous_scope_) {
        previous_scope_->Set(s, object);
    } else {
        throw NameError("Unknown identifier: " + s);
    }
}