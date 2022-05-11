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

/**
 * @brief Namespace encompassing all abstract syntax tree nodes
 *        which describe literals values and pointers to non-terminal
 *        expressions.
 * 
 */
namespace ast
{

/**
 * @brief Superclass of every single AST node.
 * 
 * @param token A Token describing type and an accompanying literal.
 */
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

/**
 * @brief Statements which are almost non-terminal and usuall call expressions.
 *  
 * @param token A token describing type and an accompanying literal.
 */
  struct Statement : public Node
  {
    Statement(Token token) : Node(token) {}

    virtual operator string() override
    {
      return token.literal;
    }
  };

/**
 * @brief Describes an interpreter command not related to SQL. 
 * 
 * @param token A token describing type and an accompanying literal.
 */
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

  /**
   * @brief Program object which runs a series of statements.
   */
  struct Program : public Node
  {
    /**
     * A series of statements to be evaluated 
     */
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

  /**
   * @brief An expression which evaluates into a terminal node. 
   */
  struct Expression : public Node
  {
    Expression(Token token) : Node(token) {}
  };

  /**
   * @brief Statement consisting of a single expression 
   */
  struct ExpressionStatement : Statement
  {
    Expression *expression;

    ExpressionStatement(Token token) : Statement(token) {}
  };

  /**
   * @brief Identiifer expression used to evaluate names, strings, and identifiers 
   */
  struct Identifier : public Expression
  {
    string value;

    Identifier(Token token, string value) : Expression(token), value(value) {}

    operator string() override
    {
      return token.literal;
    }
  };

  /**
   * @brief Integer literal expression used to evaluate integer literals for evaluation.
   */
  struct IntegerLiteral : public Expression
  {
    int value;

    IntegerLiteral(Token token, int64_t value) : Expression(token), value(value) {}

    operator string() override
    {
      return token.literal;
    }
  };

  /**
   * @brief Prefix expression used to evaluate single expressions against more expression(s).
   */
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

  /**
   * @brief Expression consisting of possibly multiple column literals
   */
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

  struct ColumnValueExpression : public Expression 
  {
    Token value;
    ColumnValueExpression *right;
    
    ColumnValueExpression(Token token) : Expression(token) {}

    string tokenLiteral() override
    {
      return token.literal;
    }

    operator string() override
    {
      ostringstream ss;
      ss << token.literal << " = " << value.literal; 
      return ss.str();
    }
  };

  struct WhereExpression : public Expression 
  {
    Token value;
    Token *value_alias = nullptr;
    Token *token_alias = nullptr;
    Token op;

    WhereExpression(Token token) : Expression(token) {}

    /**
     * @brief Gets token literal of node
     * 
     * @return string Name of column chosen
     */
    string tokenLiteral() override
    {
      return token.literal;
    }

    operator string() override 
    {
      ostringstream ss;
      ss << token.literal << " " << op.literal << " " << value.literal << std::endl;
    }
  };

  /**
   * @brief Gets identifier list with possible aliases
   * name is saved into 'token'
   * alias is saved into 'alias'
   * the next values are saved into right as a linked list
   */
  struct TableIdentifierList : public Expression
  {
      Token *alias = nullptr;
      TableIdentifierList *right = nullptr;

      TableIdentifierList(Token token) : Expression(token)
      {
      }

      string tokenLiteral() override
      {
        return token.literal;
      }

      operator string() override
      {
        ostringstream ss;
        ss << token.literal << " : ";
        if (alias != nullptr) {
          ss << std::string(*alias);
        }
        if (right != nullptr) {
          ss << ", " << std::string(*right);
        }
      }
  };

  /**
   * @brief token is full or inner
   * remaining vars are trivial
   */
  struct JoinExpression : public Expression 
  {
    // (LEFT OR RIGHT)
    Token *include;
    Token *join_ident;
    Token *join_alias;
    WhereExpression *where;
    
    JoinExpression(Token token) : Expression(token) {}

    string tokenLiteral() override
    {
      return token.literal;
    }

    operator string() override
    {
      ostringstream ss;
      return ss.str();
    }
  };

  struct ColumnQueryExpression : public Expression
  {
    ColumnQueryExpression *right = nullptr;

    ColumnQueryExpression(Token token) : Expression(token) {}

    string tokenLiteral() override
    {
      return "QUERY";
    }

    operator string() override
    {
      ostringstream ss;
      ss << token.literal;
      if (right != nullptr) {
        ss << ", " << std::string(*right);
      }
      return ss.str();
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

  struct CommitStatement : public Statement
  {
    CommitStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override 
    {
      return "COMMIT";
    }

    operator string() override
    {
      return "COMMIT;";
    }
  };

  struct BeginTransactionStatement : public Statement
  {
    BeginTransactionStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override 
    {
      return "TRANSACTION";
    }

    operator string() override
    {
      return "BEGIN TRANSACTION;";
    }
  };
  
  struct SelectTableStatement : public Statement
  {
    TableIdentifierList *names;
    ColumnQueryExpression *column_query;
    JoinExpression *join_expr = nullptr;
    WhereExpression *query = nullptr;

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
      ss << std::string(*column_query) << " FROM TABLE ";
      ss << std::string(*names);
      if (join_expr) {
        if (query)
          ss << std::string(*query);
      } else {
        if (query) {
          ss << " WHERE";
          ss << std::string(*query);
        }
      }
      ss << ";";
      return ss.str();
    }
  };

  /**
   * @brief Form: UPDATE {table} SET {column-value expr} WHERE {where-condition expr}
   */
  struct UpdateTableStatement : public Statement 
  {
    Identifier *name;
    ColumnValueExpression *column_value;
    WhereExpression *query;

    UpdateTableStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "UPDATE";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "UPDATE ";
      ss << std::string(*name);
      ss << " SET " << std::string(*column_value);
      ss << " WHERE " << std::string(*query) << ";";
      return ss.str();
    }
  };

  struct DeleteTableStatement : public Statement
  {
    Identifier *name;
    WhereExpression *query;

    DeleteTableStatement(Token token) : Statement(token)
    {
    }

    string tokenLiteral() override
    {
      return "DELETE";
    }

    operator string() override
    {
      ostringstream ss;
      ss << "DELETE FROM ";
      ss << std::string(*name);
      ss << " WHERE " << std::string(*query) << ";";
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
