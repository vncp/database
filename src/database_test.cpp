#include <gtest/gtest.h>
#include <tokens.hpp>
#include <lexer.hpp>
#include <ast.hpp>
#include <parser.hpp>
#include <string>

TEST(LexerTest, ReadNextTokenSingleChar) {
  std::string input = "(),;";

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
  std::string input = "CREATE DATABASE db_1;\
    DROP DATABASE db_2;\
    USE db_1;\
    CREATE TABLE tbl_1(a1 int, a2 varchar(20));\
    .EXIT";
  
  std::vector<Token> expectedTokens;
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
  }
}

TEST(ParserTest, CreateDatabaseStatements) {
  std::string input = "CREATE DATABASE db_1;\
                  CREATE DATABASE db_2;";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 2);

  auto testCreateDatabaseStatement = [](ast::Statement *statement, std::string name) {
    ast::CreateDatabaseStatement *casted_statement = dynamic_cast<ast::CreateDatabaseStatement*>(statement);
    EXPECT_EQ(casted_statement->tokenLiteral(), "CREATE");
    EXPECT_EQ(casted_statement->name->value, name);
    EXPECT_EQ(casted_statement->name->tokenLiteral(), name);
  };

  testCreateDatabaseStatement(program->statements[0], "db_1");
  testCreateDatabaseStatement(program->statements[1], "db_2");
}

TEST(ParserTest, CreateTableStatements) {
  std::string input = "CREATE TABLE tbl_1 (a1 int, a2 varchar(20));\
                  CREATE TABLE tbl_2(a3 float, a4 char);";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 2);

  auto testCreateTableStatement = [](ast::Statement *statement, std::string name) {
    ast::CreateTableStatement *casted_statement = dynamic_cast<ast::CreateTableStatement*>(statement);
    EXPECT_EQ(casted_statement->tokenLiteral(), "CREATE");
    EXPECT_EQ(casted_statement->name->value, name);
    EXPECT_EQ(casted_statement->name->tokenLiteral(), name);
  };

  testCreateTableStatement(program->statements[0], "tbl_1");
  testCreateTableStatement(program->statements[1], "tbl_2");
}

TEST(ParserTest, IdentifierExpressions) {
  std::string input = "applesauce;";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::ExpressionStatement *statement = dynamic_cast<ast::ExpressionStatement*>(program->statements[0]);
  ast::Identifier *expression = dynamic_cast<ast::Identifier*>(statement->expression);
  EXPECT_EQ(expression->value, "applesauce");
  EXPECT_EQ(expression->tokenLiteral(), "applesauce");
  

}