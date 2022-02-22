#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <ast.hpp>
#include <tokens.hpp>
#include <lexer.hpp>

class SQLParser {
public:
  Lexer *lexer;
  Token currToken;
  Token peekToken;

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
        nextToken();
        return parseCreateDatabaseStatement();
      }
    }
    return nullptr;
  }

  ast::CreateDatabaseStatement *parseCreateDatabaseStatement() {
    ast::CreateDatabaseStatement *statement = new ast::CreateDatabaseStatement{currToken};
    if (peekToken.type == token_type::IDENTIFIER) {
      nextToken();
    } else {
      // Not identifier error
      return nullptr;
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    while (currToken.type == token_type::SEMICOLON) {
      nextToken();
    }
    return statement;
  }
};

#endif /* __PARSER_HPP__ */