#ifndef __PROTO_GENERATOR__
#define __PROTO_GENERATOR__

#include <iostream>
#include <string>
#include <algorithm>
#include <string_view>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>
#include <data_objs.hpp>

namespace fs = std::experimental::filesystem;
// Fieldname , <Type, Count, fieldNum, sqlType>
using fieldmapType = std::unordered_map<std::string, std::tuple<std::string, int, int, std::string>>;

namespace paths
{
  const std::string PROTO_VERSION = "syntax = \"proto3\";";
  auto PROTOC_PATH = fs::current_path() / "etc" / "protobuf" / "src" / "protoc";
  auto PROJECT_ROOT = fs::current_path();
  auto DATA_PATH = fs::current_path() / "data";
};
using namespace paths;

std::unordered_map<std::string, std::string> typeMap = {
    {"int", "int64"},
    {"char", "string"},
    {"varchar", "string"},
    {"float", "float"}};

class ProtoGenerator
{
  DatabaseObject *db_obj;

  void verifyProtoc()
  {
    if (!fs::exists(PROTOC_PATH))
    {
      std::cout << "This project depends on Google Protocol Buffers to store data as bytes.\n"
                << "The application will now download, build, and install the protobuf\n"
                << "compiler and proto-cpp in the local application directory (~20 mins).\n"
                << "CTRL-C to exit or Enter to accept.\n";
      int a;
      cin >> a;
      system((PROJECT_ROOT / "install_protoc.sh").c_str());
    }
    else
    {
      std::cout << "Found protoc binary at " << PROTOC_PATH << endl;
    }
    if (!fs::exists(PROTOC_PATH))
    {
      cerr << "Could not install protoc." << endl;
    }
    // Make sure out directory for protoc exists
    fs::create_directories(PROJECT_ROOT / "include" / "generated");
  }

  // Create a new Table
  void protocGenerate(DatabaseObject *database, TableObject table)
  {
    if (!fs::exists(DATA_PATH / database->name()))
    {
      fs::create_directories(DATA_PATH / database->name());
    }
    std::ofstream protoFile(DATA_PATH / database->name() / (table.name() + ".proto"));
    protoFile << PROTO_VERSION << "\n";
    protoFile << "package " << database->name() << ";\n\n";
    protoFile << generateMetadataComment(table.maxFieldNum, table.name(), database->name());
    protoFile << "message " << table.name() << " {\n";
    for (auto field : table.fields)
    {
      protoFile << "\t // " << get<1>(field.second) << " " << get<3>(field.second) << "\n";
      protoFile << "\t" << get<0>(field.second) << " " << field.first << " = " << get<2>(field.second) << ";\n";
    }
    protoFile << "}\n";
    protoFile.flush();
    protoFile.close();
  }

  static std::string generateMetadataComment(const int maxNumField,
                                             std::string_view tableName,
                                             std::string_view databaseName)
  {
    std::ostringstream ss;
    ss << "/*    METADATA-START\n";
    ss << "databaseName " << databaseName << std::endl;
    ss << "tableName " << tableName << std::endl;
    ss << "maxFieldNum " << maxNumField << std::endl;
    ss << "METADATA-END    */\n";
    return ss.str();
  }

public:
  ProtoGenerator(DatabaseObject *db_obj) : db_obj(db_obj)
  {
    // ProtocolBuffer not needed until data stored
    // verifyProtoc();
    auto dbNamePath = DATA_PATH / db_obj->name();
    if (!fs::exists(dbNamePath))
    {
      fs::create_directories(dbNamePath);
    }
    for (auto table : db_obj->tables)
    {
      // If there's not a file for a table, create the table
      if (!fs::exists(dbNamePath / (table.name() + ".proto")))
      {
        protocGenerate(db_obj, table);
      }
    }
  }

  static bool DBExists(std::string name)
  {
    return fs::exists(DATA_PATH / name);
  }

  static bool createDB(std::string db_name)
  {
    if (fs::exists(DATA_PATH / db_name))
    {
      return false;
    }
    return fs::create_directory(DATA_PATH / db_name);
  }

  static DatabaseObject createTBL(std::string db_name, std::string tbl_name, fieldmapType fields)
  {
    auto db_path = DATA_PATH / db_name;
    // Table already exists
    if (fs::exists(db_path / (tbl_name + ".proto")))
    {
      return DatabaseObject("nil");
    }
    TableObject tbl(tbl_name);
    for (const auto &field : fields)
    {
      auto second = field.second;
      tbl.addField(field.first, typeMap[get<0>(second)], get<2>(second), get<1>(second), get<3>(second));
    }
    DatabaseObject curr_db(db_name);
    curr_db.insertTable(tbl);
    ProtoGenerator pg(&curr_db);
    return curr_db;
  }

  static std::string printTBL(std::string db_name, std::string tbl_name)
  {
    DatabaseObject db = loadDB(db_name);
    for (int i = 0; i < db.tables.size(); i++)
    {
      if (db.tables[i].name() == tbl_name)
      {
        fieldmapType fields = db.tables[i].fields;
        std::ostringstream ss;
        ss << "| ";
        for (auto [name, field] : fields)
        {
          ss << name << get<3>(field) << (get<1>(field) > 1 ? ("(" + to_string(get<1>(field)) + ") |") : " |");
        }
        return ss.str();
      }
    }
    return "Table " + tbl_name + " doesn't exist";
  }

  static DatabaseObject addFieldTBL(std::string db_name, std::string tbl_name, std::string fieldName, std::string fieldCount, std::string fieldType)
  {
    DatabaseObject db = loadDB(db_name);
    for (int i = 0; i < db.tables.size(); i++)
    {
      if (db.tables[i].name() == tbl_name)
      {
        db.tables[i].addField(fieldName, typeMap[fieldType], atoi(fieldCount.c_str()), fieldType);
        return db;
      }
    }
    return DatabaseObject("nil");
  }

  static bool deleteDB(std::string db_name)
  {
    return !fs::remove_all(DATA_PATH / db_name);
  }

  static bool deleteTBL(std::string db_name, std::string tbl_name)
  {
    return !fs::remove(DATA_PATH / db_name / (tbl_name + ".proto"));
  }

  static DatabaseObject loadDB(std::string db_name)
  {
    DatabaseObject res(db_name);
    auto db_path = (DATA_PATH / db_name);
    for (const auto &file : fs::directory_iterator(db_path))
    {
      auto proto_path = file.path();
      std::string databaseName = "";
      std::string tableName = "";
      int maxFieldNum = 0;

      std::ifstream db_file(proto_path);
      std::string prev = "";
      std::string curr = "";
      std::string next = "";
      while (curr != "databaseName")
      {
        prev = curr;
        curr = next;
        db_file >> next;
      }
      databaseName = next;
      while (curr != "tableName")
      {
        prev = curr;
        curr = next;
        db_file >> next;
      }
      tableName = next;
      while (curr != "maxFieldNum")
      {
        prev = curr;
        curr = next;
        db_file >> next;
      }
      maxFieldNum = stoi(next.c_str());
      // Now read in message fields to table
      TableObject tbl(tableName);
      std::string sqlcount = "";
      std::string sqltype = "";
      std::string type = "";
      while (next != "}")
      {
        while (curr != "=")
        {
          sqlcount = sqltype;
          sqltype = type;
          type = prev;
          prev = curr;
          curr = next;
          db_file >> next;
        }
        maxFieldNum = std::max(stoi(next), maxFieldNum);
        tbl.addField(prev, type, stoi(next.c_str()), stoi(sqlcount.c_str()), sqltype);
        sqlcount = sqltype;
        sqltype = type;
        type = prev;
        prev = curr;
        curr = next;
        db_file >> next;
      }
      res.insertTable(tbl);
      db_file.close();
    }
  }
};

#endif /* __PROTO_GENERATOR__ */