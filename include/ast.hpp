#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

namespace ast {

  struct Node {
    Node() {}
    virtual string tokenLiteral() = 0;
  };

  struct Statement : public Node {
    Statement() {}
    virtual string tokenLiteral() {return "";}
  };

  struct Expression : public Node {
    Expression() {}
    virtual string tokenLiteral() = 0;
  };

  struct Program : public Node {
    vector<Statement*> statements;

    string tokenLiteral() override {
      if (statements.size() > 0) {
        return statements[0]->tokenLiteral();
      }
      return "";
    }
  };

  struct Identifier : public Expression {
    Token token;
    string value;
    Identifier(Token token, string value) : token(token), value(value) {}

    string tokenLiteral() override {
      return token.literal;
    }
  };

  struct CreateDatabaseStatement : public Statement {
    Token token; // late token_type::CREATE
    Identifier *name;

    CreateDatabaseStatement(Token token) : token(token) {}

    string tokenLiteral() {
      return token.literal;
    }
  };

};
#endif /* __AST_HPP__ */