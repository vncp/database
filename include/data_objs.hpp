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

class TableObject {
public:
  int maxFieldNum;
  std::string table_name;
  //fieldname, TUPLE: (type, int<1> count, int<2> fieldNum)
  std::unordered_map<std::string, std::tuple<std::string, int, int>> fields;

  TableObject(std::string name, int maxFieldNum = 0) :
    table_name(name), 
    maxFieldNum(maxFieldNum) { }

  std::string name() const {
    return table_name;
  }

  bool addField(std::string name, std::string type_name, int count = 1) {
    if (fields.find(name) != fields.end()) {
      return false;
    }
    fields[name] = std::make_tuple(type_name, count, ++maxFieldNum);
  }

  // Check if a field exists
  bool has(std::string field) const {
    return fields.find(field) != fields.end();
  }
};

class DatabaseObject {
private:
  std::string database_name;
public:
  std::vector<TableObject> tables;
  DatabaseObject(std::string name) : database_name(name) {
  }

  std::string name() const {
    return database_name;
  }

  std::string tableName(const int idx) const {
    return tables[idx].name();
  }

  void insertTable(const TableObject table) {
    tables.push_back(table);
  }
};

#endif /* __DATA_OBJS__ */