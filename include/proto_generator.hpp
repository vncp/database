/*
 * AUTHOR: VINCENT PHAM
 * CLASS: CS457 DATABASE MANAGEMENT SYSTEMS
 * FILE DESC: The main function for generating, reading, and saving into
 * .proto files. The files generated from here will be used to compile
 * shared object libraries with dynamic linkage.
 */
#ifndef __PROTO_GENERATOR__
#define __PROTO_GENERATOR__

#include <iostream>
#include <string>
#include <tuple>
#include <algorithm>
#include <string_view>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>
#include <data_objs.hpp>

namespace fs = std::experimental::filesystem;
// Fieldname , <Type, Count, fieldNum, sqlType>
using fieldmapType = std::unordered_map<std::string, std::tuple<std::string, int>>;

namespace paths
{
  const std::string PROTO_VERSION = "syntax = \"proto3\";";
  auto PROTOC_PATH = fs::current_path() / "etc" / "protobuf" / "src" / "protoc";
  auto PROJECT_ROOT = fs::current_path();
  auto DATA_PATH = fs::current_path() / "data";
};
using namespace paths;

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

  // Create a new Table. Called on refresh
  void protocGenerate(DatabaseObject *database, TableObject table)
  {
    if (!fs::exists(DATA_PATH / database->name()))
    {
      fs::create_directories(DATA_PATH / database->name());
    }
    std::ofstream protoFile(DATA_PATH / database->name() / (table.name() + ".proto"));
    protoFile << PROTO_VERSION << "\n";
    protoFile << "package " << database->name() << ";\n\n";
    protoFile << generateMetadataComment(table.name(), database->name());
    protoFile << "message " << table.name() << " {\n";
    for (auto field : table.fields)
    {
      protoFile << "\t" <<  field.first << " = " << get<0>(field.second) << " " << get<1>(field.second) << "\n";
    }
    protoFile << "}\n";
    protoFile.flush();
    protoFile.close();
  }

  static std::string generateMetadataComment(std::string tableName,
                                             std::string databaseName)
  {
    std::ostringstream ss;
    ss << "/*    METADATA-START\n";
    ss << "databaseName " << databaseName << std::endl;
    ss << "tableName " << tableName << std::endl;
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
      protocGenerate(db_obj, table);
    }
  }

  static bool DBExists(std::string name)
  {
    return fs::exists(DATA_PATH / name);
  }

  static bool createDB(std::string db_name)
  {
    if(!fs::exists(DATA_PATH)) {
      fs::create_directories(DATA_PATH);
    }
    if (fs::exists(DATA_PATH / db_name))
    {
      return false;
    }
    return fs::create_directory(DATA_PATH / db_name);
  }

  // Create table object from fieldmapType
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
      tbl.addField(field.first, get<0>(second), get<1>(second));
    }
    DatabaseObject curr_db(db_name);
    curr_db.insertTable(tbl);
    ProtoGenerator pg(&curr_db);
    return curr_db;
  }

  // Converts a databse and tables to a readable format
  static std::string printTBL(std::string db_name, std::string tbl_name)
  {
    DatabaseObject db = loadDB(db_name);
    for (int i = 0; i < db.tables.size(); i++)
    {
      if (db.tables[i].name() == tbl_name)
      {
        fieldmapType fields = db.tables[i].fields;
        std::ostringstream ss;
        ss << "|";
        for (auto [name, field] : fields)
        {
          ss << " " << name << " " << get<0>(field);
          ss << (get<1>(field) > 1 ? ("(" + to_string(get<1>(field)) + ") |") : " |");
        }
        return ss.str();
      }
    }
    return "!Failed to query tbl_1 because it does not exist.";
  }

  //add a field to an existing table
  static DatabaseObject addFieldTBL(std::string db_name, std::string tbl_name, std::string fieldName, std::string fieldCount, std::string fieldType)
  {
    DatabaseObject db = loadDB(db_name);
    for (int i = 0; i < db.tables.size(); i++)
    {
      if (db.tables[i].name() == tbl_name)
      {
        db.tables[i].addField(fieldName, fieldType, atoi(fieldCount.c_str()));
        // Memory instance updated, now apply to file. Current will be updated after return
        ProtoGenerator pg(&db);
        return db;
      }
    }
    return DatabaseObject("nil");
  }

  // Delete all files then remove the directory
  static bool deleteDB(std::string db_name)
  {
    fs::remove_all(DATA_PATH / db_name);
    return !fs::remove(DATA_PATH);
  }

  // Deletes a specific table from the database's path
  static bool deleteTBL(std::string db_name, std::string tbl_name)
  {
    return !fs::remove(DATA_PATH / db_name / (tbl_name + ".proto"));
  }

  // Load database and parse table into the database and table classes
  static DatabaseObject loadDB(std::string db_name)
  {
    DatabaseObject res(db_name);
    auto db_path = (DATA_PATH / db_name);

    // Read metadata
    std::string databaseName = "";
    std::string tableName = "";
    for (const auto &file : fs::directory_iterator(db_path))
    {
      auto proto_path = file.path();

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

      // Now read in message field to table and close file before we get to the actual data
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
        // TODO: Rewrite field parser
        //tbl.addField(prev, type, stoi(next.c_str()), stoi(sqlcount.c_str()), sqltype);
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
    return res;
  }
};

#endif //__PROTO_GENERATOR__