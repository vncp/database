/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: Provides an Abstract Syntax Tree structure.
 * Due to static typing, nodes are identified by their tokenLiteral()
 * function which should be unique for every node.
 */

#ifndef __AST_HPP__
#define __AST_HPP__

#include <vector>
#include <sstream>
#include <tokens.hpp>
#include <lexer.hpp>

using namespace std;

namespace ast
{

  struct Node
  {
    Token token;
    Node(Token token) : token(token) {}
    virtual string tokenLiteral()
    {
      return token.literal;
    };
    virtual operator string() = 0;
  };

  struct Statement : public Node
  {
    Statement(Token token) : Node(token) {}

    virtual operator string() override
    {
      return token.literal;
    }
  };

  struct CommandStatement : public Statement
  {
    CommandStatement(Token token) : Statement(token) {}

    virtual operator string() override
    {
      return token.literal;
    }

    string tokenLiteral() override
    {
      return token.literal;
    }
  };

  struct Program : public Node
  {
    vector<Statement *> statements;

    Program() : Node(Token()) {}

    string tokenLiteral() override
    {
      return "PROGRAM";
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
  };

  struct Expression : public Node
  {
    Expression(Token token) : Node(token) {}
  };

  struct ExpressionStatement : Statement
  {
    Expression *expression;

    ExpressionStatement(Token token) : Statement(token) {}
  };

  struct Identifier : public Expression
  {
    string value;

    Identifier(Token token, string value) : Expression(token), value(value) {}

    operator string() override
    {
      return token.literal;
    }
  };

  struct IntegerLiteral : public Expression
  {
    int value;

    IntegerLiteral(Token token, int64_t value) : Expression(token), value(value) {}

    operator string() override
    {
      return token.literal;
    }
  };

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
  };

  // Expression consisting of possibly multiple column literals
  struct ColumnLiteralExpression : public Expression {
    // The type that's associated with the literal (read at runtime)
    Token token_vartype;

    ColumnLiteralExpression *right;
    
    ColumnLiteralExpression(Token token, Token type) : Expression(token),
                                                       token_vartype(type) {}

    string tokenLiteral() override 
    {
      return token.literal;
    }

    operator string() override
    {
      ostringstream ss;
      ss << token.literal;
      return ss.str();
    }
  };

  // Expression consisting of possibly multiple column definiitions
  struct ColumnDefinitionExpression : public Expression
  {
    Token token_vartype;
    IntegerLiteral *count;
    ColumnDefinitionExpression *right;

    ColumnDefinitionExpression(Token token, Token type) : Expression(token),
                                                          token_vartype(type) {}

    string tokenLiteral() override
    {
      return token.literal;
    }

    operator string() override
    {
      ostringstream ss;
      ss << token.literal << " " << token_vartype.literal;
      if (count != nullptr)
      {
        ss << "(" << std::string(*count) << ")";
      }
      return ss.str();
    }
  };

  struct QueryExpression : public Expression
  {
    // TODO: ColumnListExpression *comlumn_list;
    QueryExpression(Token token) : Expression(token) {}

    string tokenLiteral() override
    {
      return "QUERY";
    }

    operator string() override
    {
      return token.literal;
    }
  };

  struct UseDatabaseStatement : public Statement
  {
    Identifier *name;

    UseDatabaseStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "USE";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "USE DATABASE ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
  };

  struct CreateDatabaseStatement : public Statement
  {
    Identifier *name;

    CreateDatabaseStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "CREATEDB";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "CREATE DATABASE ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
  };

  struct CreateTableStatement : public Statement
  {
    Identifier *name;
    ColumnDefinitionExpression *column_list;

    CreateTableStatement(Token token) : Statement(token) {}

    string tokenLiteral() override
    {
      return "CREATETBL";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "CREATE TABLE ";
      ss << std::string(*name) << "(";
      ColumnDefinitionExpression *curr;
      for (curr = column_list; column_list->right != nullptr; column_list = column_list->right)
      {
        ss << std::string(*column_list) << ", ";
      }
      ss << std::string(*column_list);
      ss << ");";
      return ss.str();
    }
  };

  struct DropDatabaseStatement : public Statement
  {
    Identifier *name;

    DropDatabaseStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "DROPDB";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "DROP DATABASE ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
  };

  struct DropTableStatement : public Statement
  {
    Identifier *name;

    DropTableStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "DROPTBL";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "DROP TABLE ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
  };

  struct AlterTableStatement : public Statement
  {
    Identifier *name;
    ColumnDefinitionExpression *column_list;

    AlterTableStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "ALTER";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "ALTER TABLE ";
      ss << std::string(*name) << "(";
      ColumnDefinitionExpression *curr;
      for (curr = column_list; column_list->right != nullptr; column_list = column_list->right)
      {
        ss << std::string(*column_list) << ", ";
      }
      ss << std::string(*column_list);
      ss << ");";
      return ss.str();
    }
  };

  struct SelectTableStatement : public Statement
  {
    Identifier *name;
    QueryExpression *query;

    SelectTableStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "SELECT";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "SELECT ";
      ss << std::string(*query) << " FROM TABLE ";
      ss << std::string(*name) << ";";
      return ss.str();
    }
  };

  struct InsertTableStatement : public Statement
  {
    Identifier *name;
    ColumnLiteralExpression *column_list;

    InsertTableStatement(Token token) : Statement(token) 
    {
    }

    string tokenLiteral() override
    {
      return "INSERT";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "INSERT INTO";
      ss << std::string(*name) << "(";
      ColumnLiteralExpression *curr;
      for (curr = column_list; column_list->right != nullptr; column_list = column_list->right)
      {
        ss << std::string(*column_list) << ", ";
      }
      ss << std::string();
      ss << ");";
      return ss.str();
    }
  };
};
#endif /* __AST_HPP__ */