#include <gtest/gtest.h>
#include <tokens.hpp>
#include <lexer.hpp>
#include <ast.hpp>
#include <parser.hpp>
#include <data_objs.hpp>
#include <proto_generator.hpp>
#include <evaluator.hpp>
#include <string>
#include <tuple>

TEST(LexerTest, ReadNextTokenSingleChar)
{
  std::string input = "(),;";

  vector<Token> expectedTokens;
  expectedTokens.emplace_back(token_type::LPAREN, "(");
  expectedTokens.emplace_back(token_type::RPAREN, ")");
  expectedTokens.emplace_back(token_type::COMMA, ",");
  expectedTokens.emplace_back(token_type::SEMICOLON, ";");

  Lexer lexer(input);
  for (int i = 0; i < expectedTokens.size(); i++)
  {
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, expectedTokens[i].type);
    EXPECT_EQ(token.literal, expectedTokens[i].literal);
  }
}

TEST(LexerTest, ReadNextTokenCode)
{
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
  for (int i = 0; i < expectedTokens.size(); i++)
  {
    Token token = lexer.nextToken();
    EXPECT_EQ(token.type, expectedTokens[i].type) << "Token type: '" << token.type << "' is not the expected: '" << expectedTokens[i].type << "'\n";
    EXPECT_EQ(token.literal, expectedTokens[i].literal) << "Token literal: '" << token.literal << "' is not the expected: '" << expectedTokens[i].literal << "'\n";
  }
}

TEST(ParserTest, CreateDatabaseStatements)
{
  std::string input = "CREATE DATABASE db_1;\
                  CREATE DATABASE db_2;";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 2);

  auto testCreateDatabaseStatement = [](ast::Statement *statement, std::string name)
  {
    ast::CreateDatabaseStatement *casted_statement = dynamic_cast<ast::CreateDatabaseStatement *>(statement);
    EXPECT_EQ(casted_statement->tokenLiteral(), "CREATEDB");
    EXPECT_EQ(casted_statement->name->value, name);
    EXPECT_EQ(casted_statement->name->tokenLiteral(), name);
  };

  testCreateDatabaseStatement(program->statements[0], "db_1");
  testCreateDatabaseStatement(program->statements[1], "db_2");
}

TEST(ParserTest, CreateTableStatements)
{
  std::string input = "CREATE TABLE tbl_1 (a1 int);\
                  CREATE TABLE tbl_2(a4 char(30));";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);

  auto testCreateTableStatement = [](ast::Statement *statement, std::string name)
  {
    ast::CreateTableStatement *casted_statement = dynamic_cast<ast::CreateTableStatement *>(statement);
    EXPECT_EQ(casted_statement->tokenLiteral(), "CREATETBL");
    EXPECT_EQ(casted_statement->name->value, name);
    EXPECT_EQ(casted_statement->name->tokenLiteral(), name);
  };

  testCreateTableStatement(program->statements[0], "tbl_1");
  testCreateTableStatement(program->statements[1], "tbl_2");
}

TEST(ParserTest, IdentifierExpressions)
{
  std::string input = "applesauce123;";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::ExpressionStatement *statement = dynamic_cast<ast::ExpressionStatement *>(program->statements[0]);
  ast::Identifier *expression = dynamic_cast<ast::Identifier *>(statement->expression);
  EXPECT_EQ(expression->value, "applesauce123");
  EXPECT_EQ(expression->tokenLiteral(), "applesauce123");
}

TEST(ParserTest, IntegerLiteralExpressions)
{
  std::string input = "215;";
  Lexer lexer(input);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();

  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::ExpressionStatement *statement = dynamic_cast<ast::ExpressionStatement *>(program->statements[0]);
  ast::IntegerLiteral *expression = dynamic_cast<ast::IntegerLiteral *>(statement->expression);
  EXPECT_EQ(expression->value, 215);
  EXPECT_EQ(expression->tokenLiteral(), "215");
}

TEST(ParserTest, PrefixExpressions)
{
  std::vector<std::tuple<std::string, std::string, int>> tests;
  tests.push_back(make_tuple("-112;", "-", 112));
  tests.push_back(make_tuple("!30;", "!", 30));

  for (auto test : tests)
  {
    Lexer lexer(get<0>(test));
    SQLParser parser(&lexer);
    ast::Program *program = parser.parseSql();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);
    ast::ExpressionStatement *statement = dynamic_cast<ast::ExpressionStatement *>(program->statements[0]);
    ast::PrefixExpression *expression = dynamic_cast<ast::PrefixExpression *>(statement->expression);
    ast::IntegerLiteral *literal = dynamic_cast<ast::IntegerLiteral *>(expression->right);
    EXPECT_EQ(expression->opsymbol, get<1>(test));
    EXPECT_EQ(literal->tokenLiteral(), std::to_string(get<2>(test)));
    EXPECT_EQ(literal->value, get<2>(test));
  }
}

TEST(ParserTest, ColumnDefinitions)
{
  std::string test = "CREATE TABLE tbl1(a1 int, a2 char(10));";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);
  ast::CreateTableStatement *statement = dynamic_cast<ast::CreateTableStatement *>(program->statements[0]);
  ast::ColumnDefinitionExpression *expression = dynamic_cast<ast::ColumnDefinitionExpression *>(statement->column_list);

  EXPECT_EQ(expression->tokenLiteral(), "a1");
  EXPECT_EQ(expression->token.literal, "a1");
  EXPECT_EQ(expression->token_vartype.literal, "int");
  EXPECT_EQ(expression->count, nullptr);
  expression = expression->right;
  EXPECT_EQ(expression->tokenLiteral(), "a2");
  EXPECT_EQ(expression->token.literal, "a2");
  EXPECT_EQ(expression->token_vartype.literal, "char");
  ASSERT_NE(expression->count, nullptr);
  EXPECT_EQ(std::string(expression->count->token.type), "INT");
  EXPECT_EQ(expression->count->token.literal, "10");
}

TEST(ParserTest, ColumnLiterals)
{
  std::string test = "INSERT INTO tbl_1 VALUES(17, 'Bob');";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::InsertTableStatement *statement = dynamic_cast<ast::InsertTableStatement *>(program->statements[0]);
  ast::ColumnLiteralExpression *expression = dynamic_cast<ast::ColumnLiteralExpression *>(statement->column_list);

  EXPECT_EQ(expression->tokenLiteral(), "17");
  EXPECT_EQ(expression->token.literal, "17");
  EXPECT_EQ(expression->token_vartype.literal, "INT");
  expression = expression->right;
  EXPECT_EQ(expression->tokenLiteral(), "Bob");
  EXPECT_EQ(expression->token.literal, "Bob");
  EXPECT_EQ(expression->token_vartype.literal, "IDENTIFIER");
}

TEST(ParserTest, ParserErrors)
{
  std::string test = "CREATE TABLE TABLE;";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  try
  {
    ast::Program *program = parser.parseSql();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);
  }
  catch (expected_token_error e)
  {
    EXPECT_EQ(string(e.what()), "Expected Token: 'IDENTIFIER' but got: 'TABLE'.");
  }

  test = "CREATE TABLE tbl1(a1 juice, a2 char(10));";
  lexer = Lexer(test);
  parser = SQLParser(&lexer);
  try
  {
    ast::Program *program = parser.parseSql();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);
  }
  catch (unknown_type_error e)
  {
    EXPECT_EQ(string(e.what()), "Unknown Type: 'juice'.");
  }

  test = "CREATE TABLE tbl1(a1 int, a2 char(+10));";
  lexer = Lexer(test);
  parser = SQLParser(&lexer);
  try
  {
    ast::Program *program = parser.parseSql();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);
  }
  catch (unassigned_parse_function_error e)
  {
    EXPECT_EQ(string(e.what()), "Parse error: no parse functions found for token:  '+'.");
  }
}


TEST(ParserTest, ParseCommands)
{
  std::string test = ".EXIT";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program = parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);
  ast::Statement *statement = program->statements[0];
  EXPECT_EQ(std::string(*statement), "EXIT");
  EXPECT_EQ(std::string(statement->token.type), "EXIT");
}

TEST(ColumnFileTest, ParseColumnFile)
{
  
}