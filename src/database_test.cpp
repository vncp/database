#include <gtest/gtest.h>
#include <tokens.hpp>
#include <lexer.hpp>
#include <string>

using namespace std;

TEST(DatabaseTest, ReadNextToken) {
  string input = "(),;";

  vector<Token> expectedTokens;
  expectedTokens.push_back(Token{token_type::LPAREN, "("});
  expectedTokens.push_back(Token{token_type::RPAREN, ")"});
  expectedTokens.push_back(Token{token_type::COMMA, ","});
  expectedTokens.push_back(Token{token_type::SEMICOLON, ";"});

  Lexer lexer(input);
  for(int i = 0; i < expectedTokens.size(); i++) {
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, expectedTokens[i].type);
    EXPECT_EQ(token.literal, expectedTokens[i].literal);
  }
}

