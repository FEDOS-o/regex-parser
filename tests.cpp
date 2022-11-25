#include "main.h"
#include <gtest/gtest.h>

static std::vector<Token> get_tokens(Lexer l) {
    std::vector<Token> ans;
    while (true) {
        ans.push_back(l.next_token());
        if (l.curr_token == END) {
            break;
        }
    }
    return ans;
}

static std::string tree_to_string(Tree *t) {
    if (t == nullptr) {
        return "";
    }
    std::string res;
    for (auto c : t->children) {
        res += tree_to_string(c);
    }
    res += t->val;
    return res;
}

TEST(lexer, chars) {
    Lexer l = Lexer("abc");
    std::vector<Token> expected = {'a', 'b', 'c', END};
    std::vector<Token> result = get_tokens(l);
    EXPECT_EQ(result, expected);
}

TEST(lexer, spec_sybbols) {
    Lexer l = Lexer("|*()|*");
    std::vector<Token> expected = {OR, KLEENE, LPAR, RPAR, OR, KLEENE, END};
    std::vector<Token> result = get_tokens(l);
    EXPECT_EQ(result, expected);
}

TEST(lexer, common_string) {
    Lexer l = Lexer("((abc*b|a)*ab(aa|b*)b)*)");
    std::vector<Token> expected = {LPAR, LPAR, 'a', 'b', 'c', KLEENE, 'b', OR, 'a', RPAR,
                                   KLEENE, 'a', 'b', LPAR, 'a', 'a', OR, 'b', KLEENE, RPAR,
                                   'b', RPAR, KLEENE, RPAR, END};
    std::vector<Token> result = get_tokens(l);
    EXPECT_EQ(result, expected);
}

TEST(lexer, fail) {
    Lexer l = Lexer("((abc*b|a)12*ab(aa|34b*)b)*)");
    EXPECT_ANY_THROW(get_tokens(l));

}

TEST(parser, simple_1) {
    Parser p("a");
    auto got = p.run();
    EXPECT_EQ(tree_to_string(got), "aepsN'NepsM'MepsE'E");
    delete got;
}

TEST(parser, simple_2) {
    Parser p("a|b");
    auto got = p.run();
    EXPECT_EQ(tree_to_string(got), "aepsN'NepsM'M|bepsN'NepsM'MepsE'EE'E");
    delete got;
}

TEST(parser, simple_3) {
    Parser p("a*");
    auto got = p.run();
    EXPECT_EQ(tree_to_string(got), "a*epsN'N'NepsM'MepsE'E");
    delete got;
}

TEST(parser, simple_4) {
    Parser p("(a)");
    auto got = p.run();
    EXPECT_EQ(tree_to_string(got), "(aepsN'NepsM'MepsE'E)epsN'NepsM'MepsE'E");
    delete got;
}

TEST(parser, simple_5) {
    Parser p("ab");
    auto got = p.run();
    EXPECT_EQ(tree_to_string(got), "aepsN'NbepsN'NepsM'MM'MepsE'E");
    delete got;
}

TEST(parser, fail_par) {
    Parser p;
    p = Parser("abc(cd*|e)|(ab|cd*(ab)ab");
    EXPECT_ANY_THROW(p.run());
    p = Parser("abc(cd*|e)|(ab|cd*))(ab)ab");
    EXPECT_ANY_THROW(p.run());
}

TEST(parser, fail_or) {
    Parser p;
    p = Parser("|abc(cd*|e)|(ab|cd*(ab)ab");
    EXPECT_ANY_THROW(p.run());
    p = Parser("abc(cd*|e|)|(ab|cd*))(ab)ab");
    EXPECT_ANY_THROW(p.run());
    p = Parser("abc(cd*||e)|(ab|cd*))(ab)ab");
    EXPECT_ANY_THROW(p.run());
}

TEST(parser, fail_kleene) {
    Parser p;
    p = Parser("*abc");
    EXPECT_ANY_THROW(p.run());
}

TEST(parser, no_fail) {
    Parser p;
    p = Parser("(abc|(cd*|ab)*|cde(fd)*)*");
    EXPECT_NO_THROW(p.run());
    p = Parser("abc(cd|(ab)*)|def|abc|(c*d*a*)*");
    EXPECT_NO_THROW(p.run());
}