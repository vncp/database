#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <functional>
#include <unordered_map>
#include <ast.hpp>
#include <tokens.hpp>
#include <lexer.hpp>

class SQLParser {
public:
  Lexer *lexer;
  Token currToken;
  Token peekToken;
  unordered_map<token_type::TokenType, function<ast::Expression*()>> prefixParseFns;

  SQLParser(Lexer *lexer) : lexer(lexer) {
    // Set currToken and peekToken
    nextToken();
    nextToken();
  };

  void nextToken() {
    currToken = peekToken;
    peekToken = lexer->nextToken();
  }

  ast::Program *parseSql() {
    ast::Program *program = new ast::Program{};
    program->statements = {};

    while (currToken.type != token_type::ENDOFFILE) {
      ast::Statement *statement = parseStatement();
      if (statement != nullptr) {
        program->statements.push_back(statement);
      }
      nextToken();
    }
    return program;
  }

  ast::Statement *parseStatement() {
    if (currToken.type == token_type::CREATE) {
      if (peekToken.type == token_type::DATABASE) {
        return parseCreateDatabaseStatement();
      } else if (peekToken.type == token_type::TABLE) {
        return parseCreateTableStatement();
      } else {
        return parseExpressionStatement();
      }
    }
    return nullptr;
  }

  ast::CreateDatabaseStatement *parseCreateDatabaseStatement() {
    ast::CreateDatabaseStatement *statement = new ast::CreateDatabaseStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER) {
      nextToken();
    } else {
      // Not identifier error
      return nullptr;
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    // Ignore everything else
    nextToken();
    if (currToken.type != token_type::SEMICOLON) {
      // Unexpected token error
      return nullptr;
    }
    return statement;
  }

  ast::CreateTableStatement *parseCreateTableStatement() {
    ast::CreateTableStatement *statement = new ast::CreateTableStatement{currToken};
    nextToken();
    if (peekToken.type == token_type::IDENTIFIER) {
      nextToken();
    } else {
      // Not identifier error
      return nullptr;
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    // Ignore Column Expression
    while (currToken.type != token_type::SEMICOLON) {
      nextToken();
    }
    return statement;
  }

  ast::ExpressionStatement *parseExpressionStatement() {
    ast::ExpressionStatement *statement = new ast::ExpressionStatement{currToken};
    statement->expression = parseExpression();
    if (peekToken.type == token_type::SEMICOLON) {
      nextToken();
    }
    return statement;
  }

  ast::Expression *parseExpression() {
    function<ast::Expression*()> prefixFn = prefixParseFns[currToken.type];
    if (prefixFn != nullptr) {
      return nullptr;
    }
    return prefixFn();
  }

};

#endif /* __PARSER_HPP__ */