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

class TableObject
{
public:
  int maxFieldNum;
  std::string table_name = "NAME_ERROR";
  // fieldname, TUPLE: (type<0>, int<1> count, int<2> fieldNum, string<3> SQLType )
  std::unordered_map<std::string, std::tuple<std::string, int, int, std::string>> fields;

  TableObject(std::string name, int maxFieldNum = 0) : table_name(name),
                                                       maxFieldNum(maxFieldNum) {}
                                                      
  std::string name() const
  {
    return table_name;
  }

  bool addField(std::string name, std::string type_name, int count, std::string sql_type)
  {
    if (fields.find(name) != fields.end())
    {
      return false;
    }
    fields[name] = std::make_tuple(type_name, count, ++maxFieldNum, sql_type);
    return true;
  }
// For reloading
  bool addField(std::string name, std::string type_name, int fieldNum, int count, std::string sqltype)
  {
    if (fields.find(name) != fields.end())
    {
      return false;
    }
    fields[name] = std::make_tuple(type_name, count, fieldNum, sqltype);
    return true;
  }

  // Check if a field exists
  bool has(std::string field)
  {
    return fields.find(field) != fields.end();
  }
};

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