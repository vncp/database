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
using fieldmapType = std::vector<std::pair<std::string, std::tuple<std::string, int>>>;

// Evaluate column definition expression node and create a fieldMapType from it
fieldmapType evalFields(ast::ColumnDefinitionExpression *node)
{
  fieldmapType res;
  int fieldNum = 1;
  while (node != nullptr)
  {
    res.emplace_back(node->tokenLiteral(), make_tuple(node->token_vartype.literal,
                                           (node->count == nullptr ? 1 : node->count->value)));
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
      if (ProtoGenerator::dropTBL(current_database->name(), std::string(*node_->name))) 
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
    // Prints the table based on the node parameters
    {"SELECT", evalFnType([](ast::Node *node, DatabaseObject *current_database){
      auto node_ = dynamic_cast<ast::SelectTableStatement*>(node);
      if (current_database->name() == "nil") {
        cout << "!Failed to select from table since no database is selected.\n";
      }

      // Literal can be asterisk or LL of items
      ast::ColumnQueryExpression *column_query = node_->column_query;
      std::vector<std::string> filter_vec;
      std::vector<std::string> *filter_ptr = &filter_vec;
      while (column_query != nullptr) {
        if (column_query->token.type == token_type::ASTERISK) {
          filter_ptr = nullptr;
          break;
        }
        filter_vec.push_back(column_query->token.literal);
        column_query = column_query->right;
      }

      ast::WhereExpression *where_query = node_->query;
      std::tuple<std::string, std::string, std::string> where_tpl;
      std::tuple<std::string, std::string, std::string> *where_ptr = nullptr;
      if (where_query != nullptr) {
        where_tpl = make_tuple(where_query->token.literal, where_query->op.literal, where_query->value.literal);
        where_ptr = &where_tpl;
      }

      cout << ProtoGenerator::printTBL(current_database->name(), std::string(*node_->name), filter_ptr, where_ptr) << endl;
      return new object::Integer(7); })},
    /**
     * @brief Connects AST nodes on INSERT statement to run insertTBL statement
     */
    {"INSERT", evalFnType([](ast::Node *node, DatabaseObject *current_database) {
      auto node_ = dynamic_cast<ast::InsertTableStatement*>(node);
      // Database check
      if (current_database->name() == "nil") {
        cout << "!Failed to insert to table since no database is selected.\n";
      }
      ast::ColumnLiteralExpression *column_list = node_->column_list;
      std::vector<std::variant<int, bool, std::string, double>> value_list;
      std::string format = "";
      while (column_list != nullptr) {
        if (column_list->token_vartype.literal == "IDENTIFIER") {
          value_list.push_back(column_list->token.literal);
        } else if (column_list->token_vartype.literal == "INT") {
          value_list.push_back(std::stoi(column_list->token.literal));
        } else if (column_list->token_vartype.literal == "FLOAT") {
          value_list.push_back(std::stod(column_list->token.literal));
        }
        column_list = column_list->right;
      }
      *current_database = ProtoGenerator::insertTBL(current_database->name(), std::string(*node_->name), value_list, format);
      if (current_database->name() == "nil") {
        std::cout << "!Insertion failed, try to reload the database using USE <database-name>.\n";
      }
      std::cout << "1 new record inserted.\n";
      return new object::Integer(1);
    })},
    /**
     * @brief Connects AST nodes on DELETE statement to run ProtoGenerator::deleteTBL
     */
    {"DELETE", evalFnType([](ast::Node *node, DatabaseObject *current_database) {
      auto node_ = dynamic_cast<ast::DeleteTableStatement*>(node);
      // Database check
      if (current_database->name() == "nil") {
        cout << "!Failed to insert to table since no database is selected.\n";
      }
      ast::WhereExpression *where_query = node_->query;
      std::tuple<std::string, std::string, std::string> where_tpl;
      std::tuple<std::string, std::string, std::string> *where_ptr;
      if (where_query != nullptr) {
        where_tpl = make_tuple(where_query->token.literal, where_query->op.literal, where_query->value.literal);
        where_ptr = &where_tpl;
      }
      int delete_count = 0;
      *current_database = ProtoGenerator::deleteTBL(current_database->name(), std::string(*node_->name), &delete_count, where_ptr);
      std::cout << delete_count << " records deleted.\n";
      return new object::Integer(1);
    })},
    /**
     * @brief Connects AST nodes on UPDATE statement to run ProtoGenerator::updateTable
     */
    {"UPDATE", evalFnType([](ast::Node *node, DatabaseObject *current_database) {
      auto node_ = dynamic_cast<ast::UpdateTableStatement*>(node);
      // Database check
      if (current_database->name() == "nil") {
        cout << "!Failed to update to table since no database is selected.\n";
      }
      ast::WhereExpression *where_query = node_->query;
      std::tuple<std::string, std::string, std::string> where_tpl;
      std::tuple<std::string, std::string, std::string> *where_ptr;
      if (where_query != nullptr) {
        where_tpl = make_tuple(where_query->token.literal, where_query->op.literal, where_query->value.literal);
        where_ptr = &where_tpl;
      }
      // Create a hashmap with column : value from expression
      ast::ColumnValueExpression *column_values = node_->column_value;
      std::unordered_map<std::string, std::string> what;
      while (column_values != nullptr) {
        what[column_values->token.literal] = column_values->value.literal;
        column_values = column_values->right;
      }
      int update_count = 0;
      *current_database = ProtoGenerator::updateTBL(current_database->name(), std::string(*node_->name), what, &update_count, where_ptr);
      std::cout << update_count << " records modified.\n";
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