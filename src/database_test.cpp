#include <gtest/gtest.h>
#include <tokens.hpp>
#include <lexer.hpp>
#include <string>

using namespace std;

TEST(LexerTest, ReadNextTokenSingleChar) {
  string input = "(),;";

  vector<Token> expectedTokens;
  expectedTokens.emplace_back(token_type::LPAREN, "(");
  expectedTokens.emplace_back(token_type::RPAREN, ")");
  expectedTokens.emplace_back(token_type::COMMA, ",");
  expectedTokens.emplace_back(token_type::SEMICOLON, ";");

  Lexer lexer(input);
  for(int i = 0; i < expectedTokens.size(); i++) {
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, expectedTokens[i].type);
    EXPECT_EQ(token.literal, expectedTokens[i].literal);
  }
}

TEST(LexerTest, ReadNextTokenCode) {
  string input = "CREATE DATABASE db_1;\
    DROP DATABASE db_2;\
    USE db_1;\
    CREATE TABLE tbl_1(a1 int, a2 varchar(20));\
    .EXIT";
  
  vector<Token> expectedTokens;
  expectedTokens.emplace_back(token_type::CREATE, "CREATE");
  expectedTokens.emplace_back(token_type::DATABASE, "DATABASE");
  expectedTokens.emplace_back(token_type::IDENTIFIER, "db_1");
  expectedTokens.emplace_back(token_type::SEMICOLON, ";");
  expectedTokens.emplace_back(token_type::DROP, "DROP");
  expectedTokens.emplace_back(token_type::DATABASE, "DATABASE");
  expectedTokens.emplace_back(token_type::IDENTIFIER, "db_2");
  expectedTokens.emplace_back(token_type::SEMICOLON, ";");
  expectedTokens.emplace_back(token_type::USE, "USE");
  expectedTokens.emplace_back(token_type::IDENTIFIER, "db_1");
  expectedTokens.emplace_back(token_type::SEMICOLON, ";");
  expectedTokens.emplace_back(token_type::CREATE, "CREATE");
  expectedTokens.emplace_back(token_type::TABLE, "TABLE");
  expectedTokens.emplace_back(token_type::IDENTIFIER, "tbl_1");
  expectedTokens.emplace_back(token_type::LPAREN, "(");
  expectedTokens.emplace_back(token_type::IDENTIFIER, "a1");
  expectedTokens.emplace_back(token_type::INT_TYPE, "int");
  expectedTokens.emplace_back(token_type::COMMA, ",");
  expectedTokens.emplace_back(token_type::IDENTIFIER, "a2");
  expectedTokens.emplace_back(token_type::VARCHAR_TYPE, "varchar");
  expectedTokens.emplace_back(token_type::LPAREN, "(");
  expectedTokens.emplace_back(token_type::INT, "20");
  expectedTokens.emplace_back(token_type::RPAREN, ")");
  expectedTokens.emplace_back(token_type::RPAREN, ")");
  expectedTokens.emplace_back(token_type::SEMICOLON, ";");
  expectedTokens.emplace_back(token_type::COMMAND, ".");
  expectedTokens.emplace_back(token_type::EXIT_CMD, "EXIT");
  Lexer lexer(input);
  for(int i = 0; i < expectedTokens.size(); i++) {
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, expectedTokens[i].type) << "Token type: '" << token.type << "' is not the expected: '" << expectedTokens[i].type << "'\n"; 
    EXPECT_EQ(token.literal, expectedTokens[i].literal) << "Token literal: '" << token.literal << "' is not the expected: '" << expectedTokens[i].literal << "'\n"; 
;
  }
}

