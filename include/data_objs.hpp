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
#include <list>
#include <variant>
#include <cstdarg>

using variant_type = std::variant<
    int,
    bool,
    std::string,
    double>;

class TableObject
{
public:
  int fields_size = 0;
  std::string table_name = "NAME_ERROR";
  // fieldname, TUPLE: (type, count)
  std::list<std::pair<std::string, std::tuple<std::string, int>>> fields;

  std::vector<variant_type> records;

  TableObject(std::string name) : table_name(name) {}
                                                      
  /**
   * @brief Gets the identifier/name of the table
   * 
   * @return std::string the identifier of the table
   */
  std::string name() const
  {
    return table_name;
  }

  /**
   * @brief Add a field to the table
   * 
   * @param name The identifier of the new field
   * @param type_name The type of the field
   * @param count [optional] The count of the field if applicable
   * @return true The field was added to the table
   * @return false The field already exists in the table
   */
  bool addField(std::string name, std::string type_name, int count = 1)
  {
    // Do not use this method for updating fields
    if (has(name)) {
      return false;
    }
    std::pair<std::string, std::tuple<std::string, int>> newField;
    newField.first = name;
    newField.second = std::make_tuple(type_name, count);
    fields.push_back(newField);
    
    // Reallocate the new field for the current recorsd
    // EX. From |int|char|float| to |int|char|float|char|
    auto records_ptr = records.begin();
    // Old Field size
    int offset = fields_size;
    for (int i = 0; records_ptr != records.end(); records_ptr++) {
      if (--offset == 0) {
        records.insert(records_ptr+1, false);
        records_ptr++;
        offset = fields_size;
      }
    }
    // Update to new Field size
    fields_size = fields.size();
    return true;
  }

  /**
   * @brief Checks if a field exists
   * 
   * @param field A string with the field's identifier
   * @return true The field exists in the table
   * @return false The field does not exist in the table
   */
  bool has(std::string field)
  {
    for (auto &pair : fields) {
      if (pair.first == field) {
        return true;
      }
    }
    return false;
  }

  std::string getFormat() {
    std::string res;
    for (auto field : fields) {
      std::string type = get<0>(field.second);
      if (type == "varchar" || type == "char") {
        res += "s";
      } else if (type == "int") {
        res += "i";
      } else if (type == "bool") {
        res += "b";
      } else if (type == "float") {
        res += "f";
      } else {
        return "";
      }
    }
    return res;
  }

  bool addRecord(char* fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    for (int i = 0; fmt[i] != '\0'; i++) {
      // Create a union to store objects
      union Union_t {
        int i;
        double f;
        bool b;
        char *s;
      } Union;

      std::string str;
      switch (fmt[i]) {
        case 'i':
          Union.i = va_arg(args, int);
          records.push_back(Union.i);
          break;
        case 'f':
          Union.f = va_arg(args, double);
          records.push_back(Union.f);
          break;
        case 's':
          Union.s = va_arg(args, char*);
          int i;
          for(i = 0; Union.s[i] != '\0'; i++) {
            str += Union.s[i];
          }
          records.push_back(str);
          break;
        case 'b':
          Union.b = va_arg(args, bool);
          records.push_back(Union.b);
          break;
        default:
          break;
      }
    }
    va_end(args);
    return true;
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