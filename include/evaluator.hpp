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
    // Checks whether table has alias or not
    // Checks whether there's a join or not
    {"SELECT", evalFnType([](ast::Node *node, DatabaseObject *current_database){
      auto node_ = dynamic_cast<ast::SelectTableStatement*>(node);
      if (current_database->name() == "nil") {
        cout << "!Failed to select from table since no database is selected.\n";
      }

      ast::TableIdentifierList *names = node_->names;
      // If true, normal select
      if (names->alias == nullptr) {
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
        // Now get where
        ast::WhereExpression *where_query = node_->query;
        std::tuple<std::string, std::string, std::string> where_tpl;
        std::tuple<std::string, std::string, std::string> *where_ptr = nullptr;
        if (where_query != nullptr) {
          where_tpl = make_tuple(where_query->token.literal, where_query->op.literal, where_query->value.literal);
          where_ptr = &where_tpl;
        }
        cout << ProtoGenerator::printTBL(current_database->name(), names->token.literal, filter_ptr, where_ptr) << endl;
      } else { // multi alias where or single alias join
        // Load references of all variables and alias
        std::vector<std::pair<std::string, std::string>> var_table;
        std::array<bool, 3> joins{false, false, false}; 
        ast::TableIdentifierList *table_ident_ptr = names;
        while (table_ident_ptr != nullptr) {
          std::string ident = table_ident_ptr->token.literal;
          std::string alias = table_ident_ptr->alias->literal;
          var_table.push_back(std::make_pair(ident, alias));
          table_ident_ptr = table_ident_ptr->right;
          joins[1] = true;
        }
        // single alias join, only one ident-alias pair was added
        if (names->right == nullptr && node_->join_expr->token.type != token_type::INNER) {
          ast::JoinExpression *join_expr = node_->join_expr;
          std::string ident = join_expr->join_ident->literal;
          std::string alias = join_expr->join_alias->literal;
          var_table.push_back(std::make_pair(ident, alias));
          joins[1] = true;
          if (join_expr->include->type == token_type::LEFT) joins[0] = true;
          if (join_expr->include->type == token_type::RIGHT) joins[2] = true;
        } else if (names->right == nullptr) { // must set as a inner join based on syntax used
          ast::JoinExpression *join_expr = node_->join_expr;
          std::string ident = join_expr->join_ident->literal;
          std::string alias = join_expr->join_alias->literal;
          var_table.push_back(std::make_pair(ident, alias));
          joins[1] = true;
        }
        // Get Where from Join or from statement
        ast::WhereExpression *where_query = (node_->query == nullptr) ? node_->join_expr->where : node_->query;
        std::tuple<std::string, std::string, std::string> where_tpl;
        std::tuple<std::string, std::string, std::string> *where_ptr = nullptr;
        if (where_query != nullptr) {
          where_tpl = make_tuple(where_query->token_alias->literal, where_query->op.literal, where_query->value_alias->literal);
          where_ptr = &where_tpl;
        }
        std::cout << ProtoGenerator::printTBLJoin(current_database->name(), where_ptr, var_table, joins) << std::endl;
      }
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
    {"TRANSACTION", evalFnType([](ast::Node *node, DatabaseObject *current_database) {
      auto node_ = dynamic_cast<ast::BeginTransactionStatement*>(node);
      if (current_database->name() == "nil") {
        cout << "!Failed to update to table since no database is selected.\n";
        return new object::Integer(1);
      }
      // Create an inner repl
        std::cout << "Transaction started.\n";
        const std::string repl_prompt = isatty(fileno(stdin)) ? "(transaction) >> " : "";
        std::string input;
        std::vector<std::string> transaction_tables;
        bool breakout = false; //Set to true once we find commit;
        // Create a sub-REPL
        do {
            std::cout << repl_prompt;
            getline(cin, input);
            Lexer lexer(input);
            SQLParser parser(&lexer);
            // Try to evaluate the program in this sub REPL.
            // If the lock exists we want to replace any table that is attempted with this lock, so we replace the name of the table
            try {
                ast::Program *program = parser.parseSql();
                // Loop through all statements in the program and try to evaluate them
                // If they are altering statements then we want to substitute x_lock.proto for them

                // Check the type of the statment here
                for (ast::Statement *statement : program->statements) {
                    if (statement->tokenLiteral() == "UPDATE") {
                        ast::UpdateTableStatement *update_stmt = dynamic_cast<ast::UpdateTableStatement *>(statement);
                        std::string table_name = std::string(*update_stmt->name);
                        // Lock the table
                        if(!ProtoGenerator::lockTbl(current_database->name(), table_name)) {
                          cout << "Error: Table Flights is locked!\n";
                        } else {
                          transaction_tables.push_back(table_name);
                          // Delegate the task to the original function performed on x_lock.proto
                          evalStatementFns[update_stmt->tokenLiteral()](update_stmt, current_database);
                        }
                    } else if (statement->tokenLiteral() == "COMMIT") {
                      if (transaction_tables.size() > 0) {
                        for (std::string table_name : transaction_tables) {
                            ProtoGenerator::commitTransaction(current_database->name(), table_name);
                        }
                        transaction_tables.clear();
                        cout << "Transaction committed.\n";
                      } else {
                        cout << "Transaction abort.\n";
                      }
                    } else {
                      evalStatementFns[statement->tokenLiteral()](statement, current_database);
                    }
                }
            }
            catch (const expected_token_error &e)
            {
            cerr << e.what() << endl;
            }
            catch (const unknown_type_error &e)
            {
            cerr << e.what() << endl;
            }
            catch (const unassigned_parse_function_error &e)
            {
            cerr << e.what() << endl;
            }
            catch (const unknown_command_error &e)
            {
            cerr << e.what() << endl;
            }
            catch (const runtime_error &e)
            {
            cerr << e.what() << endl;
            }
        } while (!breakout);


      return new object::Integer(0);
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