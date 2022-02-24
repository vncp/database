#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>
#include <sstream>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

namespace ast {

  struct Node {
    Token token;
    Node(Token token) : token(token) {}
    virtual string tokenLiteral() {
      return token.literal;
    };
    virtual operator string() = 0;
  };

  struct Statement : public Node {
    Statement(Token token) : Node(token) {}
    virtual operator string() {return "";}
  };

  struct Program : public Node {
    vector<Statement*> statements;

    Program() : Node(Token()) {}

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

  struct Expression : public Node {
    Expression(Token token) : Node(token) {}
  };

  struct ExpressionStatement : Statement {
    Expression *expression;

    ExpressionStatement(Token token): Statement(token) {}
  };

  struct Identifier : public Expression {
    string value;

    Identifier(Token token, string value) : Expression(token), value(value) {}

    operator string() override {
      return token.literal;
    }
  };

  struct IntegerLiteral : public Expression {
    int value;

    IntegerLiteral(Token token, int64_t value) : Expression(token), value(value) {}

    operator string() override {
      return token.literal;
    }
  };

  struct PrefixExpression : public Expression {
    Token token;
    string opsymbol;
    Expression *right;

    PrefixExpression(Token token, string opsymbol) : 
      Expression(token), 
      opsymbol(opsymbol) {}

    operator string() override {
      ostringstream ss;
      ss << "(" << opsymbol << std::string(*right) << ")";
      return ss.str();
    }
  };

  struct ColumnDefinitionExpression : public Expression {
    Token token_vartype;
    IntegerLiteral *count;
    ColumnDefinitionExpression *right;

    ColumnDefinitionExpression(Token token, Token type) : 
      Expression(token),
      token_vartype(type) {}
    
    string tokenLiteral() override{
      return token.literal + "_" +  token_vartype.literal;
    }

    operator string() override {
      ostringstream ss;
      ss << "(" << token.literal << " " << token_vartype.literal;
      ss << "[" << std::string(*count) << "])";
      ss << string(*right);
      return ss.str();
    }
  };

  struct CreateDatabaseStatement : public Statement {
    Identifier *name;

    CreateDatabaseStatement(Token token) : Statement(token) {
    }

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
    Identifier *name;
    ColumnDefinitionExpression *column_list;

    CreateTableStatement(Token token) : Statement(token) {}

    string tokenLiteral() override {
      return token.literal;
    }

    operator string() override {
      ostringstream ss;
      ss << tokenLiteral() << " ";
      ss << std::string(*name);
      if (column_list != nullptr) {
        ss << std::string(*column_list);
      }
      ss << ";";
      return ss.str();
    }
  };

};
#endif /* __AST_HPP__ */