#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>
#include <sstream>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

namespace ast {

  struct Node {
    Node() {}
    virtual string tokenLiteral() = 0;
    virtual operator string() = 0;
  };

  struct Statement : public Node {
    Statement() {}
    virtual string tokenLiteral() {return "";}
    virtual operator string() {return "";}
  };

  struct Expression : public Node {
    Expression() {}
     virtual string tokenLiteral() override {return "";};
  };

  struct ExpressionStatement : Statement {
    Token token;
    Expression *expression;

    ExpressionStatement(Token token): token(token) {}

    string tokenLiteral() override {
      return token.literal;
    }
  };

  struct Program : public Node {
    vector<Statement*> statements;

    string tokenLiteral() override {
      if (statements.size() > 0) {
        return statements[0]->tokenLiteral();
      }
      return "";
    }

    operator string() override {
      ostringstream ss;
      for (const auto &statement : statements) {
        ss << string(*statement);
      }
      return ss.str();
    }
  };

  struct Identifier : public Expression {
    Token token;
    string value;

    Identifier(Token token, string value) : token(token), value(value) {}

    string tokenLiteral() override {
      return token.literal;
    }

    operator string() override {
      return token.literal;
    }
  };

  struct IntegerLiteral : public Expression {
    Token token;
    int value;

    IntegerLiteral(Token token, int64_t value) : token(token), value(value) {}

    string tokenLiteral() override {
      return token.literal;
    }

    operator string() override {
      return token.literal;
    }
  };

  struct PrefixExpression : public Expression {
    Token token;
    string opsymbol;
    Expression *right;

    PrefixExpression(Token token, string opsymbol) : 
      token(token), 
      opsymbol(opsymbol) {}

    string tokenLiteral() override {
      return token.literal;
    }

    operator string() override {
      ostringstream ss;
      ss << "(" << opsymbol << std::string(*right) << ")";
      return ss.str();
    }
  };

  struct CreateDatabaseStatement : public Statement {
    Token token; // late token_type::CREATE
    Identifier *name;

    CreateDatabaseStatement(Token token) : token(token) {}

    string tokenLiteral() override {
      return token.literal;
    }

    operator string() override {
      ostringstream ss;
      ss << tokenLiteral() << " ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
  };

  struct CreateTableStatement : public Statement {
    Token token;
    Identifier *name;
    Expression *value;

    CreateTableStatement(Token token) : token(token) {}

    string tokenLiteral() {
      return token.literal;
    }

    operator string() override {
      ostringstream ss;
      ss << tokenLiteral() << " ";
      ss << std::string(*name);
      if (value != nullptr) {
        ss << std::string(*value);
      }
      ss << ";";
      return ss.str();
    }
  };

};
#endif /* __AST_HPP__ */