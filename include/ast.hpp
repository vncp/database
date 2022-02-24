#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>
#include <sstream>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

namespace ast
{

  struct NodeTag;
  struct Node
  {
    Token token;
    Node(Token token) : token(token) {}
    virtual string tokenLiteral()
    {
      return token.literal;
    };
    virtual operator string() = 0;
    using tag = NodeTag;
  };

  struct StatementTag;
  struct Statement : public Node
  {
    Statement(Token token) : Node(token) {}

    virtual operator string() override
    {
      return token.literal;
    }
    using tag = StatementTag;
  };

  struct ProgramTag;
  struct Program : public Node
  {
    vector<Statement *> statements;

    Program() : Node(Token()) {}

    string tokenLiteral() override
    {
      if (statements.size() > 0)
      {
        return statements[0]->tokenLiteral();
      }
      return "";
    }

    operator string() override
    {
      ostringstream ss;
      for (const auto &statement : statements)
      {
        ss << string(*statement);
      }
      return ss.str();
    }
    using tag = ProgramTag;
  };

  struct ExpressionTag;
  struct Expression : public Node
  {
    Expression(Token token) : Node(token) {}
  };

  struct ExpressionStatementTag;
  struct ExpressionStatement : Statement
  {
    Expression *expression;

    ExpressionStatement(Token token) : Statement(token) {}
    using tag = ExpressionTag;
  };

  struct IdentifierTag;
  struct Identifier : public Expression
  {
    string value;

    Identifier(Token token, string value) : Expression(token), value(value) {}

    operator string() override
    {
      return token.literal;
    }
    using tag = IdentifierTag;
  };

  struct IntegerLiteralTag;
  struct IntegerLiteral : public Expression
  {
    int value;

    IntegerLiteral(Token token, int64_t value) : Expression(token), value(value) {}

    operator string() override
    {
      return token.literal;
    }
    using tag = IntegerLiteralTag;
  };

  struct PrefixExpressionTag;
  struct PrefixExpression : public Expression
  {
    string opsymbol;
    IntegerLiteral *right;

    PrefixExpression(Token token, string opsymbol) : Expression(token),
                                                     opsymbol(opsymbol) {}

    operator string() override
    {
      ostringstream ss;
      ss << "(" << opsymbol << std::string(*right) << ")";
      return ss.str();
    }
    using tag = PrefixExpressionTag;
  };

  struct ColumnDefinitionExpressionTag;
  struct ColumnDefinitionExpression : public Expression
  {
    Token token_vartype;
    IntegerLiteral *count;
    ColumnDefinitionExpression *right;

    ColumnDefinitionExpression(Token token, Token type) : Expression(token),
                                                          token_vartype(type) {}

    string tokenLiteral() override
    {
      return token.literal + "_" + token_vartype.literal + (count ? ("(" + to_string(count->value) + ")") : "");
    }

    operator string() override
    {
      ostringstream ss;
      ss << token.literal << token_vartype.literal;
      if (count != nullptr)
      {
        ss << "(" << std::string(*count) << ")";
      }
      return ss.str();
    }
    using tag = ColumnDefinitionExpressionTag;
  };

  struct CreateDatabaseStatementTag;
  struct CreateDatabaseStatement : public Statement
  {
    Identifier *name;

    CreateDatabaseStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return token.literal;
    }

    operator string() override
    {
      ostringstream ss;
      ss << tokenLiteral() << " DATABASE ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
    using tag = CreateDatabaseStatementTag;
  };

  struct CreateTableStatementTag;
  struct CreateTableStatement : public Statement
  {
    Identifier *name;
    ColumnDefinitionExpression *column_list;

    CreateTableStatement(Token token) : Statement(token) {}

    string tokenLiteral() override
    {
      return token.literal;
    }

    operator string() override
    {
      ostringstream ss;
      ss << tokenLiteral() << " TABLE ";
      ss << std::string(*name) << "[";
      if (column_list != nullptr)
      {
        ss << std::string(*column_list);
      }
      ss << ";";
      return ss.str();
    }
    using tag = CreateTableStatementTag;
  };
};
#endif /* __AST_HPP__ */