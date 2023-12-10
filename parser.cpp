#include "parser.h"
#include "tokenizer.h"
#include "error.h"

ObjectPtr ReadList(Tokenizer* tokenizer);

ObjectPtr Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
    Token current_token = tokenizer->GetToken();
    tokenizer->Next();
    if (current_token == Token{BracketToken{BracketToken::CLOSE}}) {
        throw SyntaxError("");
    } else if (current_token == Token{BracketToken{BracketToken::OPEN}}) {
        return ReadList(tokenizer);
    } else if (current_token == Token{QuoteToken{}}) {
        if (tokenizer->IsEnd() ||
            tokenizer->GetToken() == Token{BracketToken{BracketToken::CLOSE}}) {
            throw SyntaxError("");
        }
        ObjectPtr cell = std::make_shared<Cell>();
        As<Cell>(cell)->GetFirst() = std::make_shared<Symbol>("quote");
        As<Cell>(cell)->GetSecond() = std::make_shared<Cell>();
        As<Cell>(As<Cell>(cell)->GetSecond())->GetFirst() = Read(tokenizer);
        return cell;
    } else if (SymbolToken* x = std::get_if<SymbolToken>(&current_token)) {
        return std::make_shared<Symbol>(x->name);
    } else if (ConstantToken* y = std::get_if<ConstantToken>(&current_token)) {
        return std::make_shared<Number>(y->value);
    } else {
        throw SyntaxError("");
    }
}

ObjectPtr ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    } else if (tokenizer->GetToken() == Token{BracketToken{BracketToken::CLOSE}}) {
        tokenizer->Next();
        return ObjectPtr();
    }

    ObjectPtr root_cell = std::make_shared<Cell>();
    ObjectPtr current_cell = root_cell;

    for (Token t = tokenizer->GetToken(); t != Token{BracketToken{BracketToken::CLOSE}};
         t = tokenizer->GetToken()) {
        ObjectPtr val = Read(tokenizer);
        As<Cell>(current_cell)->GetFirst() = val;
        if (tokenizer->IsEnd()) {
            throw SyntaxError("");
        }
        if (tokenizer->GetToken() == Token{DotToken{}}) {  // pair
            tokenizer->Next();
            ObjectPtr second = Read(tokenizer);
            As<Cell>(current_cell)->GetSecond() = second;
            if (tokenizer->IsEnd() ||
                tokenizer->GetToken() != Token{BracketToken{BracketToken::CLOSE}}) {
                throw SyntaxError("");
            }
            tokenizer->Next();
        } else if (tokenizer->GetToken() == Token{BracketToken{BracketToken::CLOSE}}) {
            As<Cell>(current_cell)->GetSecond() = ObjectPtr();
            tokenizer->Next();
            break;
        } else {
            As<Cell>(current_cell)->GetSecond() = std::make_shared<Cell>();
            current_cell = As<Cell>(current_cell)->GetSecond();
        }
    }

    return root_cell;
}
