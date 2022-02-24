#ifndef __DATA_OBJS__
#define __DATA_OBJS__

#include <string>
#include <tuple>
#include <vector>
#include <variant>

using variant_type = std::variant<
  int,
  bool,
  std::nullptr_t,
  std::string,
  double>;

class TableObject {
  // fieldname, fieldtype, count (if fieldtype == VARCHAR or CHAR)
  std::vector<std::tuple<std::string, variant_type, int>> fields;

};

class DatabaseObject {
  std::vector<TableObject> tables;
};

#endif /* __DATA_OBJS__ */