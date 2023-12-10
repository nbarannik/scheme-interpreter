#pragma once

#include <unordered_map>
#include <string>
#include "object.h"
#include "error.h"

class Scope {
public:
    void InitGlobalScope();
    void Define(const std::string& s, ObjectPtr object);
    void Set(const std::string& s, ObjectPtr object);
    ObjectPtr Get(const std::string& s);
    std::shared_ptr<Scope> GetPreviousScope();
    void SetPreviousScope(std::shared_ptr<Scope> other);

private:
    std::unordered_map<std::string, ObjectPtr> registered_functions_;
    std::shared_ptr<Scope> previous_scope_;

    friend void SetCurrentScope(std::shared_ptr<Scope> other);
};

std::shared_ptr<Scope> GetCurrentScope();
void SetCurrentScope(std::shared_ptr<Scope> other);