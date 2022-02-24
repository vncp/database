#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <functional>
#include <unordered_map>
#include <string>
#include <ast.hpp>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

enum class precedence {
  LOWEST,
  EQUALS,
  LESS_GREATER,
  SUM,
  PRODUCT,
  CALL
};

std::unordered_map<token_type::TokenType, precedence> precedences = {
  {token_type::EQ, precedence::EQUALS},
  {token_type::NE, precedence::EQUALS},
  {token_type::LT, precedence::LESS_GREATER},
  {token_type::GT, precedence::LESS_GREATER},
  {token_type::PLUS, precedence::SUM},
  {token_type::MINUS, precedence::SUM},
  {token_type::SLASH, precedence::PRODUCT},
  {token_type::ASTERISK, precedence::PRODUCT}
};

using exprParseFnType = function<ast::Expression*()>;

class SQLParser {
public:
  Lexer *lexer;
  Token currToken;
  Token peekToken;
  unordered_map<token_type::TokenType, exprParseFnType*> prefixParseFns;

  // Define parsing functions
  exprParseFnType parseIdentifier = exprParseFnType([&, this]() {
    return new ast::Identifier{currToken, currToken.literal};
  });
  exprParseFnType parseIntegerLiteral = exprParseFnType([&, this]() {
    try {
      int value = std::stoi(currToken.literal);
      return new ast::IntegerLiteral{currToken, value};
    } catch (const std::invalid_argument &ia) {
      return static_cast<ast::IntegerLiteral*>(nullptr);
    }
  });
  exprParseFnType parsePrefixExpression = exprParseFnType([&, this]() {
    ast::PrefixExpression *expr = new ast::PrefixExpression{currToken, currToken.literal};
    // Move on to expression after prefix
    nextToken();
    expr->right = parseExpression();
    return expr;
  });

  SQLParser(Lexer *lexer) : lexer(lexer) {
    // Set currToken and peekToken
    nextToken();
    nextToken();

    // Map tokens to prefix parse functions
    prefixParseFns[token_type::IDENTIFIER] = &parseIdentifier;
    prefixParseFns[token_type::INT] = &parseIntegerLiteral;
    prefixParseFns[token_type::BANG] = &parsePrefixExpression;
    prefixParseFns[token_type::MINUS] = &parsePrefixExpression;
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
      }
    } else {
      return parseExpressionStatement();
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
      cerr << "Expected Token: IDENTIFIER, Got: " << peekToken.type << endl;
      return nullptr;
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    // Ignore everything else
    nextToken();
    if (currToken.type != token_type::SEMICOLON) {
      // Unexpected token error
      cerr << "Unexpected Token. CreateDatabaseStatement should end after identifier." << endl;
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
      cerr << "Expected Token: IDENTIFIER, Got: " << peekToken.type << endl;
      return nullptr;
    }
    statement->name = new ast::Identifier{currToken, currToken.literal};
    cout << statement->tokenLiteral() << " TABLE" << endl;
    cout << "Table name: " << string(*statement->name) << endl;

    nextToken();
    if (currToken.type != token_type::LPAREN) {
      cerr << "Expected Token: LPAREN, Got: " << currToken.type << endl;
      return nullptr;
    }
    nextToken();
    statement->column_list = parseColumnDefinition();

    nextToken();
    if (currToken.type != token_type::RPAREN) {
      cerr << "Expected closing parentheses after column list" << endl;
      return nullptr;
    }
    nextToken();
    if (currToken.type != token_type::SEMICOLON) {
      cerr << "Unexpected Token CreateTableStatement should have semicolon after column list." << endl;
      return nullptr;
    }
    return statement;
  }

  ast::ColumnDefinitionExpression *parseColumnDefinition() {
    auto expr = new ast::ColumnDefinitionExpression{currToken, peekToken};
    nextToken();
    token_type::TokenType varType = token_type::lookUpType(currToken.literal);
    if (varType == token_type::TYPE) {
      cerr << "Undefined type: " << currToken.literal << endl;
      return nullptr;
    }
    if (varType == token_type::VARCHAR_TYPE || varType == token_type::CHAR_TYPE) {
      nextToken();
      if (currToken.type != token_type::LPAREN) {
        cerr << "Expected '('. Got " << string(currToken.type) << endl;;
      }
      expr->count = dynamic_cast<ast::IntegerLiteral*>(parseIntegerLiteral());
      if (currToken.type != token_type::RPAREN) {
        cerr << "Expected ')'. Got: " << string(currToken.type) << endl;
      }
    } else {
      expr->count = static_cast<ast::IntegerLiteral*>(nullptr);
    }
    if (peekToken.type == token_type::COMMA) {


    }
    return expr;
  }

  ast::ExpressionStatement *parseExpressionStatement() {
    ast::ExpressionStatement *statement = new ast::ExpressionStatement{currToken};
    statement->expression = parseExpression();
    if (peekToken.type == token_type::SEMICOLON) {
      nextToken();
    }
    return statement;
  }

  ast::Expression *parseExpression(int precedence = 0) {
    if (prefixParseFns.find(currToken.type) != prefixParseFns.end()){
      exprParseFnType *prefixFn = prefixParseFns[currToken.type];
      if (prefixFn != nullptr) {
        return (*prefixFn)();
      }
    } else {
      cerr << "No prefix parse function for token: " << currToken.type << endl;
    }
    return nullptr;
  }


};

#endif /* __PARSER_HPP__ */