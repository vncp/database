/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: Evaluates statements into native language components
 * and executes via proto_generator. It takes the AST as input.
 * Some expressins may recursively be evaluated.
 */

#ifndef __EVALUATOR_HPP__
#define __EVALUATOR_HPP__

#include <typeinfo>
#include <any>
#include <string>
#include <functional>
#include <objects.hpp>
#include <data_objs.hpp>
#include <proto_generator.hpp>
#include <ast.hpp>

object::Object *eval(ast::Node *node, DatabaseObject *current_database);

// Fieldname , <Type, Count>
using fieldmapType = std::unordered_map<std::string, std::tuple<std::string, int>>;

// Evaluate column definition expression node and create a fieldMapType from it
fieldmapType evalFields(ast::ColumnDefinitionExpression *node)
{
  fieldmapType res;
  int fieldNum = 1;
  while (node != nullptr)
  {
    res[node->tokenLiteral()] = make_tuple(node->token_vartype.literal,
                                           (node->count == nullptr ? 1 : node->count->value));
    node = node->right;
  }
  return res;
}

// Typedef for std::function parameters
using evalFnType = std::function<object::Object *(ast::Node *, DatabaseObject *)>;
// Map token identifiers with equivalent runtimes
std::unordered_map<std::string, evalFnType> evalStatementFns = {
    // Create a table using ProtoGenerator::createTBL based on fields evaluated using evalFields()
    {"CREATETBL", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                             {
      auto node_ = dynamic_cast<ast::CreateTableStatement*>(node);
      fieldmapType fields = evalFields(node_->column_list);
      if (current_database->name() == "nil") 
      {
        cout << "Not currently using any database.\n";
        return new object::Integer(1);
      }
      DatabaseObject new_db = ProtoGenerator::createTBL(current_database->name(), std::string(*node_->name), fields);
      if(new_db.name() != "nil") 
      {
        //*current_database = ProtoGenerator::loadDB(current_database->name());
        cout << "Table " << std::string(*node_->name) << " created.\n";
        return new object::Integer(0); 
      }
      cout << "!Failed to create " << std::string(*node_->name) << " because it already exists.\n";
      return new object::Integer(1); })},
    // Create a database using  current database object node information
    {"CREATEDB", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                            {
      auto node_ = dynamic_cast<ast::CreateDatabaseStatement*>(node);
      if(ProtoGenerator::createDB(std::string(*node_->name))){
        cout << "Database " << std::string(*node_->name) << " created.\n";
        return new object::Integer(0); 
      }
      cout << "!Failed to create " << std::string(*node_->name) << " as it already exists.\n";
      return new object::Integer(1); })},
    // Drop table in the current database
    {"DROPTBL", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                           {
      auto node_ = dynamic_cast<ast::DropTableStatement*>(node);
      if (current_database->name() == "nil") 
      {
        cout << "!Failed to delete " << std::string(*node_->name) << " because no database was selected.\n";
        return new object::Integer(1);
      }
      if (ProtoGenerator::deleteTBL(current_database->name(), std::string(*node_->name))) 
      {
        cout << "!Failed to delete " << std::string(*node_->name) << " because it does not exist.\n";
        return new object::Integer(2);
      }
      cout << "Table " << std::string(*node_->name) << " deleted.\n";
      return new object::Integer(0); })},
    // Deletes database directory
    {"DROPDB", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                          {
      auto node_ = dynamic_cast<ast::DropDatabaseStatement*>(node);
      if (ProtoGenerator::deleteDB(std::string(*node_->name))) 
      {
        cout << "-- !Failed to delete " << std::string(*node_->name) << " because it does not exist.\n";
        return new object::Integer(2);
      }
      cout << "Database " << std::string(*node_->name) << " deleted.\n";
      return new object::Integer(0); })},
    // Select the currently used database
    {"USE", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                       {
      auto node_ = dynamic_cast<ast::UseDatabaseStatement*>(node);
      if(ProtoGenerator::DBExists(*node_->name)) 
      {
        cout << "Using database " << std::string(*node_->name) << ".\n";
        *current_database = ProtoGenerator::loadDB(std::string(*node_->name));
        return new object::Integer(0);
      } 
      cout << "!Failed to use " << std::string(*node_->name) << " because it doesn't exist.\n";
      return new object::Integer(1); })},
    // Alter a table
    {"ALTER", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                         {
      auto node_ = dynamic_cast<ast::AlterTableStatement*>(node);
      if (current_database->name() == "nil") 
      {
        cout << "!Failed to alter " << std::string(*node_->name) << " because no database was selected.\n";
        return new object::Integer(1);
      }
      ast::ColumnDefinitionExpression *column_def = node_->column_list;
      DatabaseObject addedFieldDb = ProtoGenerator::addFieldTBL(
        current_database->name(),
        std::string(*node_->name), 
        column_def->tokenLiteral(), 
        (column_def->count != nullptr ? ("(" + column_def->count->tokenLiteral() + ")") : ""), 
        column_def->token_vartype.literal).name();
      if (addedFieldDb.name() != "nil") 
      {
        *current_database = ProtoGenerator::loadDB(current_database->name());
        cout <<"Table " << std::string(*node_->name) << " modified.\n";
        return new object::Integer(0);
      }
      cout << "!Could not modify or find table.\n";
      return new object::Integer(1); 
    })},
    // Prints the table
    {"SELECT", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                          {
      auto node_ = dynamic_cast<ast::SelectTableStatement*>(node);
      if (current_database->name() == "nil") {
        cout << "!Failed to query since no databases are being used.\n";
      }
      cout << ProtoGenerator::printTBL(current_database->name(), std::string(*node_->name)) << endl;
      return new object::Integer(7); })},
    {"INSERT", evalFnType([](ast::Node *node, DatabaseObject *current_database) {
      auto node_ = dynamic_cast<ast::InsertTableStatement*>(node);
      // Database check
      if (current_database->name() == "nil") {
        cout << "!Failed to insert to table since no database is selected.\n";
      }
      // Table check

      return new object::Integer(1);
    })},
    // Program statement
    {"PROGRAM", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                           {
      auto node_ = dynamic_cast<ast::Program*>(node);
      object::Object *ret;
      for (auto &stmt : node_->statements) 
      {
        ret = eval(stmt, current_database);
      }
      return ret; })},
    // Exit program
    {"EXIT", evalFnType([](ast::Node *node, DatabaseObject *current_database)
                        {
      auto node_ = dynamic_cast<ast::Program*>(node);
      cout << "All done.\n";
      exit(EXIT_SUCCESS);
      return new object::Integer(0); })},
};

object::Object *eval(ast::Node *node, DatabaseObject *current_database)
{
  if (evalStatementFns.find(node->tokenLiteral()) != evalStatementFns.end())
  {
    return evalStatementFns[node->tokenLiteral()](node, current_database);
  }
  else
  {
    return new object::Integer(-1);
  }
}

#endif /* __EVALUATOR_HPP__ */