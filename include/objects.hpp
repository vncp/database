/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: Objects created by the evaluator. Mainly for
 * later use when there are arithmetic and equality expressions.
 * For now, only the integer object is used in order to give returns.
 */
#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <string>
#include <sstream>
#include <iostream>
#include <ast.hpp>

namespace ast {
  struct Node;
  struct Statement;
  struct Program;
  struct Expression;
  struct Identifier;
  struct IntegerLiteral;
  struct PrefixExpression;
  struct ColumnDefinitionExpression;
  struct CreateDatabaseStatement;
  struct CreateTableStatement;
  struct InsertTableStatement;
};

namespace object
{
  enum ObjectType : char
  {
    PROGRAM_OBJ,
    INTEGER_OBJ,
    CREATE_TBL_OBJ,
  };

  struct Object
  {
    Object() {}
    virtual ObjectType type() const = 0;
    virtual std::string inspect() const = 0;
  };

  struct ProgramObject : public Object
  {
    ast::Statement *statements;

    ProgramObject(ast::Statement *statements) : statements(statements) {}

    ObjectType type() const override
    {
      return PROGRAM_OBJ;
    }

    std::string inspect() const
    {
      return "PROGRAM";
    }
  };

  struct CreateTableObject : public Object
  {
    ast::Identifier *name;
    ast::ColumnDefinitionExpression *columns;

    CreateTableObject(ast::Identifier *name, ast::ColumnDefinitionExpression *columns) : name(name), columns(columns) {}

    ObjectType type() const override
    {
      return CREATE_TBL_OBJ;
    }

    std::string inspect() const
    {
      std::ostringstream ss;
      ss << "CREATE TABLE ";
      ss << std::string(*name) << "(";
      ast::ColumnDefinitionExpression *currColumn;
      for (currColumn = columns; currColumn->right != nullptr; currColumn = currColumn->right)
      {
        ss << std::string(*currColumn) << ", ";
      }
      ss << std::string(*currColumn) << ")";
      return ss.str();
    }
  };

  struct Integer : public Object
  {
    int64_t value;

    Integer(int64_t val) : value(val) {}

    std::string inspect() const override
    {
      return std::to_string(value);
    }

    ObjectType type() const
    {
      return INTEGER_OBJ;
    }
  };

};

#endif /* __OBJECTS_H__ */