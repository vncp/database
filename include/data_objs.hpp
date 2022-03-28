/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: A simple object representation of the data needed
 * to generate and translate between protobuf files and program memory.
 */
#ifndef __DATA_OBJS__
#define __DATA_OBJS__

#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <variant>

using variant_type = std::variant<
    int,
    bool,
    std::nullptr_t,
    std::string,
    double>;

// Simple table struct to represent schemas in C++
class TableObject
{
public:
  // fieldname, TUPLE: (sql_type, var_count)
  std::unordered_map<std::string, std::tuple<std::string, int>> fields;

  TableObject(std::string name, int maxFieldNum = 0) : table_name(name){}
                                                      
  std::string name() const
  {
    return table_name;
  }

  // Adds field to table object, returns false if the variable already exists
  bool addField(std::string name, std::string type_name, const int count)
  {
    if (fields.find(name) != fields.end())
    {
      return false;
    }
    fields[name] = std::make_tuple(type_name, count);
    return true;
  }

class DatabaseObject
{
private:
  std::string database_name;

public:
  std::vector<TableObject> tables;

  DatabaseObject(std::string name) : database_name(name)
  {
  }

  std::string name() const
  {
    return database_name;
  }

  void insertTable(TableObject table)
  {
    tables.push_back(table);
  }
};

#endif /* __DATA_OBJS__ */
