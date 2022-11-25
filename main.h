#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include "graphviz/gvc.h"

/*
 * E->ME'
 * E'->|E
 * E'->eps
 * M->NM'
 * M'->M
 * M'->eps
 * N->(E)N'
 * N->\charN'
 * N'->*N'
 * N'->eps
 * */




struct parsing_exception : std::exception {};

enum Token_type {
    LPAR,
    RPAR,
    END,
    OR,
    KLEENE,
    CHAR
};

struct Token {
    Token_type type;
    char val;

    Token(Token_type type) : type(type), val(0) {}

    Token(char val) : type(CHAR), val(val) {}

    bool operator==(const Token& other) const {
        return type == other.type && val == other.val;
    }

    bool operator!=(const Token& other) const {
        return !(*this==other);
    }
};

std::ostream& operator<<(std::ostream &out, Token t) {
    auto [type, val] = t;
    switch (type) {
        case CHAR:
            out << "CHAR(" << val << ")";
            break;
        case LPAR:
            out << "(";
            break;
        case RPAR:
            out << ")";
            break;
        case OR:
            out << "|";
            break;
        case KLEENE:
            out << "*";
            break;
        case END:
            out << "EOF";
            break;
    }
    return out;
}

struct Lexer {

    Lexer() : curr_token(0) {}

    explicit Lexer(std::string s) : curr_token(0) {
        src = std::move(s);
        curr = 0;
    }

    static bool is_ws(char c) {
        return c == ' ' || c == '\r' || c == '\n' || c == '\t';
    }

    void skip_ws() {
        while (curr != src.size() && is_ws(src[curr])) curr++;
    }

    bool is_empty() {
        return curr == src.size();
    }

    Token next_token() {
        skip_ws();
        if (curr == src.size()) {
            curr_token = END;
            return END;
        }
        char c = src[curr];
        curr++;
        switch (c) {
            case '(':
                curr_token = LPAR;
                return LPAR;
            case ')':
                curr_token = RPAR;
                return RPAR;
            case '*':
                curr_token = KLEENE;
                return KLEENE;
            case '|':
                curr_token = OR;
                return OR;
            default:
                if ('a' <= c && c <= 'z') {
                    curr_token = c;
                    return c;
                } else {
                    std::cerr << "\nunexpected symbol: '" <<  c << "' at pos " << curr << " in \"" << src << "\"\n";
                    throw parsing_exception();
                }
        }
    }

    Token curr_token;
private:
    std::string src;
    size_t curr;
};

struct Tree {
    size_t num;
    std::string val;
    std::vector<Tree*> children;

    Tree(std::string s, size_t& num) : val(std::move(s)), num(num++) {}

    Tree(std::string s, std::vector<Tree*> children) : val(std::move(s)), children(std::move(children)) {}

    ~Tree() {
        for (auto c : children) {
            delete c;
        }
    }

    std::string convert_to_dot() {
        std::stringstream ss;
        ss << num << " [label=\"" << val << "\"]\n";
        if (children.empty()) {

        }
        for (auto c : children) {
            ss << num << " -> " << c->num << '\n';
            ss << c->convert_to_dot();
        }
        return ss.str();
    }
};

struct Parser {

    explicit Parser() {}

    explicit Parser(std::string s) : lexer(std::move(s)) {}

    void expected(const std::vector<Token>& set) const {
        std::cerr << "expected: ";
        for (auto t : set) {
            if (t.type != CHAR) {
                std::cerr << "'" << t << "', ";
            } else {
                std::cerr << "char, ";
            }
        }
        std::cerr << "\ngot: '" << lexer.curr_token << "'" << '\n';
    }

    Tree* E() {
        Tree* res = new Tree("E", node_num);
        switch (lexer.curr_token.type) {
            case CHAR:
            case LPAR:
                res->children.push_back(M());
                res->children.push_back(E_prime());
                break;
            default:
                expected({LPAR, CHAR});
                throw parsing_exception();
        }
        return res;
    }

    Tree* E_prime() {
        Tree* res = new Tree("E'", node_num);
        switch (lexer.curr_token.type) {
            case OR:
                res->children.push_back(new Tree("|", node_num));
                lexer.next_token();
                res->children.push_back(E());
                break;
            case RPAR:
            case END:
            case KLEENE:
                res->children.push_back(new Tree("eps", node_num));
                break;
            default:
                expected({OR, RPAR, END});
                throw parsing_exception();

        }
        return res;
    }

    Tree* M() {
        Tree* res = new Tree("M", node_num);
        switch (lexer.curr_token.type) {
            case CHAR:
            case LPAR:
                res->children.push_back(N());
                res->children.push_back(M_prime());
                break;
            default:
                expected({LPAR, CHAR});
                throw parsing_exception();
        }
        return res;
    }

    Tree* M_prime() {
        Tree* res = new Tree("M'", node_num);
        switch (lexer.curr_token.type) {
            case LPAR:
            case CHAR:
                res->children.push_back(M());
                break;
            case OR:
            case RPAR:
            case END:
            case KLEENE:
                res->children.push_back(new Tree("eps", node_num));
                break;
            default:
                expected({LPAR, CHAR, OR, RPAR, END});
                throw parsing_exception();
        }
        return res;
    }

    Tree* N() {
        Tree* res = new Tree("N", node_num);
        switch (lexer.curr_token.type) {
            case CHAR:
                res->children.push_back(new Tree(std::string(1, lexer.curr_token.val), node_num));
                lexer.next_token();
                res->children.push_back(N_prime());
                break;
            case LPAR:
                res->children.push_back(new Tree("(", node_num));
                lexer.next_token();
                res->children.push_back(E());
                if (lexer.curr_token.type != RPAR) {
                    expected({RPAR});
                    throw parsing_exception();
                }
                res->children.push_back(new Tree(")", node_num));
                lexer.next_token();
                res->children.push_back(N_prime());
                break;
            default:
                expected({LPAR, CHAR});
                throw parsing_exception();
        }
        return res;
    }

    Tree* N_prime() {
        Tree* res = new Tree("N'", node_num);
        switch (lexer.curr_token.type) {
            case KLEENE:
                res->children.push_back(new Tree("*", node_num));
                lexer.next_token();
                res->children.push_back(N_prime());
                break;
            case OR:
            case RPAR:
            case END:
            case CHAR:
            case LPAR:
                res->children.push_back(new Tree("eps", node_num));
                break;
            default:
                expected({KLEENE, OR, LPAR, RPAR, END, CHAR});
                throw parsing_exception();
        }
        return res;
    }

    Tree* run() {

        lexer.next_token();
        node_num = 0;
        auto res = E();
        if (lexer.curr_token.type != END) {
            expected({END});
            throw parsing_exception();
        }
        return res;
    }

private:
    Lexer lexer;
    size_t node_num;
};

