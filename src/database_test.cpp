/**
 * @file database_test.cpp
 * @author Vincent Pham
 * @brief Unit tests for lexer, parser, evaluator, and Cxx-style objects.
 * @version 2.1
 */

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
#include <variant>

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

TEST(ParserTest, UpdateStatement_WhereExpressions_ColumnValueExpressions){
  std::string test = "UPDATE product \nSET price = 14.99 \nWHERE name = 'Gizmo';";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::UpdateTableStatement *statement = dynamic_cast<ast::UpdateTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::ColumnValueExpression *column_values = statement->column_value;
  ast::WhereExpression *where_expression = statement->query;

  EXPECT_EQ(table_name->tokenLiteral(), "product");
  EXPECT_EQ(column_values->token.literal, "price");
  EXPECT_EQ(column_values->value.literal, "14.99");
  EXPECT_EQ(where_expression->token.literal, "name");
  EXPECT_EQ(where_expression->op.literal, "=");
  EXPECT_EQ(where_expression->value.literal, "Gizmo");
}

TEST(ParserTest, DeleteStatement)
{
  std::string test = "DELETE FROM product where name = 'Gizmo';";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::DeleteTableStatement *statement = dynamic_cast<ast::DeleteTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::WhereExpression *delete_query = statement->query;

  EXPECT_EQ(statement->token.literal, "DELETE");
  EXPECT_EQ(table_name->token.literal, "product");
  EXPECT_EQ(delete_query->token.literal, "name");
  EXPECT_EQ(delete_query->op.literal, "=");
  EXPECT_EQ(delete_query->value.literal, "Gizmo");
}

TEST(ParserTest, WhereExpressionNumeric) {
  std::string test = "DELETE FROM product where price < 150;";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::DeleteTableStatement *statement = dynamic_cast<ast::DeleteTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::WhereExpression *delete_query = statement->query;

  EXPECT_EQ(statement->token.literal, "DELETE");
  EXPECT_EQ(table_name->token.literal, "product");
  EXPECT_EQ(delete_query->token.literal, "price");
  EXPECT_EQ(delete_query->op.literal, "<");
  EXPECT_EQ(delete_query->value.literal, "150");
}

TEST(ParserTest, SelectStatement_ColumnUnion)
{
  std::string test = "SELECT name, price FROM product;";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::SelectTableStatement *statement = dynamic_cast<ast::SelectTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::ColumnQueryExpression *column_query = statement->column_query;

  EXPECT_EQ(statement->token.literal, "SELECT");
  EXPECT_EQ(table_name->token.literal, "product");
  EXPECT_EQ(column_query->token.literal, "name");
  ASSERT_NE(column_query->right, nullptr);
  column_query = column_query->right;
  EXPECT_EQ(column_query->token.literal, "price");
}

TEST(ParserTest, SelectStatement_SingleColumn)
{
  std::string test = "SELECT name FROM product;";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::SelectTableStatement *statement = dynamic_cast<ast::SelectTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::ColumnQueryExpression *column_query = statement->column_query;

  EXPECT_EQ(statement->token.literal, "SELECT");
  EXPECT_EQ(table_name->token.literal, "product");
  EXPECT_EQ(column_query->token.literal, "name");
  EXPECT_EQ(column_query->right, nullptr);
}

TEST(ParserTest, SelectStatement_Asterisk)
{
  std::string test = "SELECT * FROM product;";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::SelectTableStatement *statement = dynamic_cast<ast::SelectTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::ColumnQueryExpression *column_query = statement->column_query;

  EXPECT_EQ(statement->token.literal, "SELECT");
  EXPECT_EQ(table_name->token.literal, "product");
  EXPECT_EQ(column_query->token.literal, "*");
  EXPECT_EQ(column_query->token.type, token_type::ASTERISK);
  EXPECT_EQ(column_query->right, nullptr);
}
TEST(ParserTest, SelectStatement_WhereExpression)
{
  std::string test = "SELECT name FROM product WHERE x = 1;";
  Lexer lexer(test);
  SQLParser parser(&lexer);
  ast::Program *program =  parser.parseSql();
  ASSERT_NE(program, nullptr);
  ASSERT_EQ(program->statements.size(), 1);

  ast::SelectTableStatement *statement = dynamic_cast<ast::SelectTableStatement *>(program->statements[0]);
  ast::Identifier *table_name = statement->name;
  ast::ColumnQueryExpression *column_query = statement->column_query;
  ast::WhereExpression *query = statement->query;

  EXPECT_EQ(statement->token.literal, "SELECT");
  EXPECT_EQ(table_name->token.literal, "product");
  EXPECT_EQ(column_query->token.literal, "name");
  EXPECT_EQ(column_query->right, nullptr);
  EXPECT_EQ(query->token.literal, "x");
  EXPECT_EQ(query->op.literal, "=");
  EXPECT_EQ(query->value.literal, "1");
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

TEST(TableTestMem, AddFieldRecords)
{
  auto table = TableObject("test_table");
  ASSERT_EQ(table.name(), "test_table");
  ASSERT_EQ(table.addField("a1", "int", 1), true);
  ASSERT_EQ(table.addField("a2", "char", 6), true);
  ASSERT_EQ(table.addField("a3", "float", 1), true);
  auto fields = table.fields.begin();
  EXPECT_EQ(get<0>(fields->second), "int");
  fields++;
  EXPECT_EQ(get<0>(fields->second), "char");
  fields++;
  EXPECT_EQ(get<0>(fields->second), "float");
  EXPECT_EQ(table.fields_size, 3);
  table.addRecord("isf", 12, "Hello", 3.64);
  // Now add a another field
  ASSERT_EQ(table.addField("a4", "varchar", 5), true);
  // Add corresponding record
  table.addRecord("isfs", 12, "Hello", 3.64, "Hello");
  table.addRecord("i", 12);
  table.addRecord("s", "Hello");
  table.addRecord("f", 3.64);
  table.addRecord("s", "Hello");
  for(auto record: table.records) {
    if(string *value = std::get_if<std::string>(&record)) {
      //std::cout << *value << std::endl;
      EXPECT_EQ(*value, "Hello");
    } else if (int *value = std::get_if<int>(&record)) {
      //std::cout << *value << std::endl;
      EXPECT_EQ(*value, 12);
    } else if (double *value = std::get_if<double>(&record)) {
      //std::cout << *value << std::endl;
      EXPECT_EQ(*value, 3.64);
    } else if (bool *value = std::get_if<bool>(&record)) {
      //std::cout << *value << std::endl;
    }
  }
}

