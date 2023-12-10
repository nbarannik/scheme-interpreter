#pragma once

#include <memory>
#include <vector>

class Object;
class Scope;

using ObjectPtr = std::shared_ptr<Object>;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::string ToString() const = 0;
    virtual ObjectPtr Evaluate() = 0;
    virtual ObjectPtr Apply(ObjectPtr);
};

class Function : public Object {
public:
    std::string ToString() const override;
    ObjectPtr Evaluate() override;
};

class Lambda : public Function {
public:
    Lambda(ObjectPtr body, const std::vector<std::string>& list, std::shared_ptr<Scope> scope)
        : body_(body), initialize_list_(list), scope_(scope) {
    }

    ObjectPtr Apply(ObjectPtr obj);

private:
    ObjectPtr body_;
    std::vector<std::string> initialize_list_;
    std::shared_ptr<Scope> scope_;
};

class Number : public Object {
public:
    Number() = default;
    Number(int64_t value) : value_(value) {
    }
    std::string ToString() const override;
    ObjectPtr Evaluate() override;
    int64_t GetValue() const;

private:
    int64_t value_;
};

class Symbol : public Object {
public:
    Symbol() = default;
    Symbol(const std::string& s) : name_(s) {
    }
    std::string ToString() const override;
    ObjectPtr Evaluate() override;
    const std::string& GetName() const;

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell() = default;
    Cell(const ObjectPtr& f, const ObjectPtr& s) : first_(f), second_(s) {
    }

    std::string ToStringInner() const;
    std::string ToString() const override;

    ObjectPtr Evaluate() override;

    ObjectPtr GetFirst() const;
    ObjectPtr GetSecond() const;

    ObjectPtr& GetFirst();
    ObjectPtr& GetSecond();

private:
    ObjectPtr first_, second_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const ObjectPtr& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const ObjectPtr& obj) {
    try {
        std::shared_ptr<T> tmp = As<T>(obj);
        return tmp.get();
    } catch (...) {
        return false;
    }
}
