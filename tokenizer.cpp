#include <cctype>
#include <string>
#include "tokenizer.h"
#include "error.h"

bool SymbolToken::operator==(const SymbolToken &other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}

bool DotToken::operator==(const DotToken &) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken &other) const {
    return value == other.value;
}

bool Tokenizer::IsEnd() const {
    return is_end_;
}

Token Tokenizer::GetToken() const {
    return last_token_;
}

static inline bool IsStartSymbol(char c) {
    return (isalpha(c) || c == '<' || c == '=' || c == '>' || c == '*' || c == '/' || c == '#');
}

static inline bool IsInnerSymbol(char c) {
    return (IsStartSymbol(c) || isdigit(c) || c == '?' || c == '!' || c == '-');
}

static ConstantToken ReadConstantToken(std::istream *in) {
    std::string number_string;
    while (isdigit(in->peek())) {
        number_string += in->get();
    }
    return ConstantToken{std::stoi(number_string)};
}

static SymbolToken ReadSymbolToken(std::istream *in) {
    std::string token_string;
    token_string += in->get();
    while (IsInnerSymbol(in->peek())) {
        token_string += in->get();
    }
    return SymbolToken{token_string};
}

void Tokenizer::Next() {
    int c;
    while (std::isspace(c = in_->peek())) {
        in_->get();
    }
    if (c == std::char_traits<char>::eof()) {
        is_end_ = true;
    } else if (c == '(' || c == ')') {
        last_token_ = BracketToken{(c == '(' ? BracketToken::OPEN : BracketToken::CLOSE)};
        in_->get();
    } else if (c == '\'') {
        last_token_ = QuoteToken();
        in_->get();
    } else if (c == '.') {
        last_token_ = DotToken();
        in_->get();
    } else if (isdigit(c)) {
        last_token_ = ReadConstantToken(in_);
    } else if (IsStartSymbol(c)) {
        last_token_ = ReadSymbolToken(in_);
    } else if (c == '+' || c == '-') {
        in_->get();
        int next_c = in_->peek();
        if (isdigit(next_c)) {
            ConstantToken token = ReadConstantToken(in_);
            if (c == '-') {
                token.value *= -1;
            }
            last_token_ = token;
        } else {
            last_token_ = SymbolToken{(c == '+' ? "+" : "-")};
        }
    } else {
        throw SyntaxError("");
    }
}

Tokenizer::Tokenizer(std::istream *in) : is_end_(false), in_(in) {
    Next();
}
