#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <functional>
#include <unordered_map>
#include <string>
#include <cstring>
#include <ast.hpp>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

enum class precedence
{
  LOWEST,
  EQUALS,
  LESS_GREATER,
  SUM,
  PRODUCT,
  CALL
};

class not_supported_error : public runtime_error
{
  std::string token;

public:
  not_supported_error(const std::string &what_arg) : runtime_error(what_arg),
                                                                                   token(what_arg) {}

  const char *what() const noexcept override
  {
    string err = ("Feature: " + token + " is currently unsupported.");
    char *err_cstr = new char[err.length() + 1];
    strcpy(err_cstr, err.c_str());
    return err_cstr;
  }
};

class expected_token_error : public runtime_error
{
  std::string token;
  std::string expected;

public:
  expected_token_error(const std::string &what_arg, const std::string &expected) : runtime_error(what_arg),
                                                                                   token(what_arg),
                                                                                   expected(expected) {}

  const char *what() const noexcept override
  {
    string err = ("Expected Token: '" + expected + "' but got: '" + token + "'.");
    char *err_cstr = new char[err.length() + 1];
    strcpy(err_cstr, err.c_str());
    return err_cstr;
  }
};

class unknown_command_error : public runtime_error
{
  std::string cmd;

public:
  unknown_command_error(const std::string &what_arg) : runtime_error(what_arg),
                                                       cmd(what_arg) {}

  virtual const char *what() const noexcept override
  {
    string err = ("Unknown Command: '" + cmd + "'.");
    char *err_cstr = new char[err.length() + 1];
    strcpy(err_cstr, err.c_str());
    return err_cstr;
  }
};

class unknown_type_error : public runtime_error
{
  std::string type;

public:
  unknown_type_error(const std::string &what_arg) : runtime_error(what_arg),
                                                    type(what_arg) {}

  virtual const char *what() const noexcept override
  {
    string err = ("Unknown Type: '" + type + "'.");
    char *err_cstr = new char[err.length() + 1];
    strcpy(err_cstr, err.c_str());
    return err_cstr;
  }
};

class unassigned_parse_function_error : public runtime_error
{
  std::string token;

public:
  unassigned_parse_function_error(const std::string &what_arg) : runtime_error(what_arg),
                                                                 token(what_arg) {}

  virtual const char *what() const noexcept override
  {
    string err = ("Parse error: no parse functions found for token:  '" + token + "'.");
    char *err_cstr = new char[err.length() + 1];
    strcpy(err_cstr, err.c_str());
    return err_cstr;
  }
};

std::unordered_map<token_type::TokenType, precedence> precedences = {
    {token_type::EQ, precedence::EQUALS},
    {token_type::NE, precedence::EQUALS},
    {token_type::LT, precedence::LESS_GREATER},
    {token_type::GT, precedence::LESS_GREATER},
    {token_type::PLUS, precedence::SUM},
    {token_type::MINUS, precedence::SUM},
    {token_type::SLASH, precedence::PRODUCT},
    {token_type::ASTERISK, precedence::PRODUCT}};

using exprParseFnType = function<ast::Expression *()>;

class SQLParser
{
public:
  Lexer *lexer;
  Token currToken;
  Token peekToken;
  unordered_map<token_type::TokenType, exprParseFnType *> prefixParseFns;

  // Define parsing functions
  exprParseFnType parseIdentifier = exprParseFnType([&, this]()
                                                    { return new ast::Identifier{currToken, currToken.literal}; });
  exprParseFnType parseIntegerLiteral = exprParseFnType([&, this]()
                                                        {
    try {
      int value = std::stoi(currToken.literal);
      return new ast::IntegerLiteral{currToken, value};
    } catch (const std::invalid_argument &ia) {
      return static_cast<ast::IntegerLiteral*>(nullptr);
    } });
  exprParseFnType parsePrefixExpression = exprParseFnType([&, this]()
                                                          {
    ast::PrefixExpression *expr = new ast::PrefixExpression{currToken, currToken.literal};
    // Move on to expression after prefix
    nextToken();
    expr->right = dynamic_cast<ast::IntegerLiteral*>(parseIntegerLiteral());
    return expr; });

  SQLParser(Lexer *lexer) : lexer(lexer)
  {
    // Set currToken and peekToken
    nextToken();
    nextToken();

    // Map tokens to prefix parse functions
    prefixParseFns[token_type::IDENTIFIER] = &parseIdentifier;
    prefixParseFns[token_type::INT] = &parseIntegerLiteral;
    prefixParseFns[token_type::BANG] = &parsePrefixExpression;
    prefixParseFns[token_type::MINUS] = &parsePrefixExpression;
  };

  void nextToken()
  {
    currToken = peekToken;
    peekToken = lexer->nextToken();
  }

  ast::Program *parseSql()
  {
    ast::Program *program = new ast::Program{};
    program->statements = {};

    while (currToken.type != token_type::ENDOFFILE)
    {
      ast::Statement *statement = parseStatement();
      if (statement != nullptr)
      {
        program->statements.push_back(statement);
      }
      nextToken();
    }
    return program;
  }

  ast::Statement *parseStatement()
  {
    if (currToken.type == token_type::COMMAND)
    {
      return parseCommand();
    }
    else if (currToken.type == token_type::CREATE)
    {
      if (peekToken.type == token_type::DATABASE)
      {
        return parseCreateDatabaseStatement();
      }
      else if (peekToken.type == token_type::TABLE)
      {
        return parseCreateTableStatement();
      }
    }
    else if (currToken.type == token_type::DROP)
    {
      if (peekToken.type == token_type::DATABASE)
      {
        return parseDropDatabaseStatement();
      }
      else if (peekToken.type == token_type::TABLE)
      {
        return parseDropTableStatement();
      }
    }
    else if (currToken.type == token_type::USE)
    {
      return parseUseDatabaseStatement();
    }
    else if (currToken.type == token_type::ALTER)
    {
      if (peekToken.type == token_type::TABLE)
      {
        return parseAlterTableStatement();
      }
    }
    else if (currToken.type == token_type::SELECT)
    {
      return parseSelectTableStatement();
    }
    else
    {
      return parseExpressionStatement();
    }
  }

  ast::Statement *parseCommand()
  {
    nextToken();
    if (token_type::lookUpCommand(currToken.literal) != token_type::COMMAND)
    {
      return new ast::Statement{currToken};
    }
    else
    {
      throw unknown_command_error(currToken.literal);
    }
  }

  ast::SelectTableStatement *parseSelectTableStatement() {
    ast::SelectTableStatement *statement = new ast::SelectTableStatement{currToken};
    nextToken();
    statement->query = parseQueryExpression();
    if (currToken.type != token_type::FROM) {
      throw expected_token_error(currToken.literal, "FROM");
    }
    nextToken();
    if (currToken.type != token_type::IDENTIFIER) {
      throw expected_token_error(currToken.type, "IDENTIFIER");
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    nextToken();
    if (currToken.type == token_type::COMMA) {
      throw expected_token_error(currToken.type, ",");
    }
    return statement;
  }

  ast::QueryExpression *parseQueryExpression() {
    if (currToken.type == token_type::ASTERISK) {
      auto expr = new ast::QueryExpression{currToken};
      nextToken();
      return expr;
    } else {
      throw not_supported_error("querying by column-list expressions");
    }
  }

  ast::UseDatabaseStatement *parseUseDatabaseStatement()
  {
    ast::UseDatabaseStatement *statement = new ast::UseDatabaseStatement{currToken};
    if (peekToken.type == token_type::IDENTIFIER)
    {
      nextToken();
      statement->name = new ast::Identifier{currToken, currToken.literal};
    }
    else
    {
      throw expected_token_error(peekToken.literal, "IDENTIFIER");
    }
    nextToken();
    if (currToken.type != token_type::SEMICOLON)
    {
      // Unexpected token error
      throw expected_token_error(currToken.literal, ";");
    }
    return statement;
  }

  ast::CreateDatabaseStatement *parseCreateDatabaseStatement()
  {
    ast::CreateDatabaseStatement *statement = new ast::CreateDatabaseStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER)
    {
      nextToken();
    }
    else
    {
      throw expected_token_error(peekToken.literal, "IDENTIFIER");
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    // Ignore everything else
    nextToken();
    if (currToken.type != token_type::SEMICOLON)
    {
      // Unexpected token error
      throw expected_token_error(currToken.literal, ";");
    }
    return statement;
  }

  ast::DropDatabaseStatement *parseDropDatabaseStatement()
  {
    ast::DropDatabaseStatement *statement = new ast::DropDatabaseStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER)
    {
      nextToken();
    }
    else
    {
      throw expected_token_error(peekToken.literal, "IDENTIFIER");
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    // Ignore everything else
    nextToken();
    if (currToken.type != token_type::SEMICOLON)
    {
      // Unexpected token error
      throw expected_token_error(currToken.literal, ";");
    }
    return statement;
  }

  ast::CreateTableStatement *parseCreateTableStatement()
  {
    ast::CreateTableStatement *statement = new ast::CreateTableStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER)
    {
      nextToken();
    }
    else
    {
      // Not identifier error
      throw expected_token_error(peekToken.literal, "IDENTIFIER");
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};

    nextToken();
    if (currToken.type != token_type::LPAREN)
    {
      throw expected_token_error(currToken.literal, "(");
    }
    nextToken();
    statement->column_list = parseColumnDefinition();
    if (currToken.type != token_type::RPAREN)
    {
      throw expected_token_error(currToken.literal, ")");
    }
    nextToken();
    if (currToken.type != token_type::SEMICOLON)
    {
      throw expected_token_error(currToken.literal, ";");
    }
    return statement;
  }

  ast::AlterTableStatement *parseAlterTableStatement()
  {
    ast::AlterTableStatement *statement = new ast::AlterTableStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER)
    {
      nextToken();
    }
    else
    {
      // Not identifier error
      throw expected_token_error(peekToken.literal, "IDENTIFIER");
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};

    nextToken();
    if (currToken.type == token_type::ADD) {
      expected_token_error(currToken.literal, "ADD");
    }
    nextToken();
    // EXPECT PARENTHESES
    if (currToken.type == token_type::LPAREN)
    {
      if (currToken.type != token_type::LPAREN)
      {
        throw expected_token_error(currToken.literal, "(");
      }
      nextToken();
      statement->column_list = parseColumnDefinition();
      if (currToken.type != token_type::RPAREN)
      {
        throw expected_token_error(currToken.literal, ")");
      }
      nextToken();
    }
    else // NO PAREN, so non-recursive parseColumnDefinition
    {
      statement->column_list = parseColumnDefinition(true);
    }
    if (currToken.type != token_type::SEMICOLON)
    {
      throw expected_token_error(currToken.literal, ";");
    }
    return statement;
  }

  ast::DropTableStatement *parseDropTableStatement()
  {
    ast::DropTableStatement *statement = new ast::DropTableStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER)
    {
      nextToken();
    }
    else
    {
      throw expected_token_error(peekToken.literal, "IDENTIFIER");
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    // Ignore everything else
    nextToken();
    if (currToken.type != token_type::SEMICOLON)
    {
      // Unexpected token error
      throw expected_token_error(currToken.literal, ";");
    }
    return statement;
  }

  ast::ColumnDefinitionExpression *parseColumnDefinition(bool single = false)
  {
    auto expr = new ast::ColumnDefinitionExpression{currToken, peekToken};
    nextToken();
    token_type::TokenType varType = token_type::lookUpType(currToken.literal);
    if (varType == token_type::TYPE)
    {

      throw unknown_type_error(currToken.literal);
    }
    if (varType == token_type::VARCHAR_TYPE || varType == token_type::CHAR_TYPE)
    {
      nextToken();
      if (currToken.type != token_type::LPAREN)
      {
        throw expected_token_error(currToken.literal, "(");
      }
      nextToken();
      expr->count = dynamic_cast<ast::IntegerLiteral *>(parseExpression());
      nextToken();
      if (currToken.type != token_type::RPAREN)
      {
        throw expected_token_error(currToken.literal, ")");
      }
    }
    else
    {
      expr->count = static_cast<ast::IntegerLiteral *>(nullptr);
    }
    nextToken();
    if (!single && currToken.type == token_type::COMMA)
    {
      nextToken();
      expr->right = parseColumnDefinition();
    }
    else
    {
      expr->right = static_cast<ast::ColumnDefinitionExpression *>(nullptr);
    }
    return expr;
  }

  ast::ExpressionStatement *parseExpressionStatement()
  {
    ast::ExpressionStatement *statement = new ast::ExpressionStatement{currToken};
    statement->expression = parseExpression();
    if (peekToken.type == token_type::SEMICOLON)
    {
      nextToken();
    }
    return statement;
  }

  ast::Expression *parseExpression(int precedence = 0)
  {
    if (prefixParseFns.find(currToken.type) != prefixParseFns.end())
    {
      exprParseFnType *prefixFn = prefixParseFns[currToken.type];
      if (prefixFn != nullptr)
      {
        return (*prefixFn)();
      }
    }
    else
    {
      throw unassigned_parse_function_error(string(currToken.type));
    }
    return nullptr;
  }
};

#endif /* __PARSER_HPP__ */