#include "main.h"
#include <fstream>
#include <vector>

static std::vector<Token> get_tokens(Lexer l) {
    std::vector<Token> ans;
    while (true) {
        Token t = l.next_token();
        ans.push_back(t);
        if (t.type == END) {
            break;
        }
    }
    return ans;
}

int main() {
    std::cout << "Enter regular expression:\n";
    std::string s;
    std::cin >> s;
    Parser p = Parser(s);
    Tree* res;
    try {
        res = p.run();
    } catch (parsing_exception& p) {
        std::cerr << "invalid regular expression";
        return -1;
    }
    std::ofstream out("graph.dot");
    out << "digraph {\n"
        << res->convert_to_dot() << "}";
    out.close();
    system("dot -Tsvg graph.dot > graph.svg");
    std::cout << "parse tree saved into graph.svg\n";
    delete res;
    return 0;
}