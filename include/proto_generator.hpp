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
  auto DATA_PATH = fs::current_path() / "data";
};
using namespace paths;


// Class responsible for generating and reading schema files
// NOTE: Originally worked with protobuf files
class ProtoGenerator
{
  DatabaseObject *db_obj;

  // Create a new Table
  void protocGenerate(DatabaseObject *database, TableObject table)
  {
    if (!fs::exists(DATA_PATH / database->name()))
    {
      fs::create_directories(DATA_PATH / database->name());
    }
    std::ofstream protoFile(DATA_PATH / database->name() / (table.name() + ".schema"));
    protoFile << generateMetadataComment(table.maxFieldNum, table.name(), database->name());
    for (auto field : table.fields)
    {
      protoFile << "\t // " << get<1>(field.second) << " " << get<3>(field.second) << "\n";
      protoFile << "\t" << get<0>(field.second) << " " << field.first << " = " << get<2>(field.second) << ";\n";
    }
    protoFile << "}\n";
    protoFile.flush();
    protoFile.close();
  }

  // Generate Metadata in top of schema file to test that they're in the correct database/table
  static std::string generateMetadataComment(const int maxNumField,
                                             std::string_view tableName,
                                             std::string_view databaseName)
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

  // Check that a database exists by checking existance of its directory
  static bool DBExists(std::string name)
  {
    return fs::exists(DATA_PATH / name);
  }

  // Create a database by creating a directory in the the DATA_PATH
  // Returns false if the directory already exists
  static bool createDB(std::string db_name)
  {
    // Make sure the DATA_PATH is created
    if(!fs::exists(DATA_PATH)) {
      fs::create_directories(DATA_PATH);
    }

    if (fs::exists(DATA_PATH / db_name))
    {
      return false;
    }
    return fs::create_directory(DATA_PATH / db_name);
  }

  // Creates a table in the provided database based on fieldmapType (unordered map)
  // Returns a database object to update state
  static DatabaseObject createTBL(std::string db_name, std::string tbl_name, fieldmapType fields)
  {
    auto db_path = DATA_PATH / db_name;
    // Table already exists
    if (fs::exists(db_path / (tbl_name + ".schema")))
    {
      return DatabaseObject("nil");
    }
    TableObject tbl(tbl_name);
    for (const auto &field : fields)
    {
      auto fieldType = field.second;
      // TODO: Update AddField
      tbl.addField(field.first, get<0>(fieldType), get<1>(fieldType));
    }
    DatabaseObject curr_db(db_name);
    curr_db.insertTable(tbl);
    ProtoGenerator pg(&curr_db);
    return curr_db;
  }

  // [STATIC] Prints table information in form of a string
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
          // Print count information only if there is more than 1 of type
          ss << (get<1>(field) > 1 ? ("(" + std::to_string(get<1>(field)) + ") |") : " |");
        }
        return ss.str();
      }
    }
    return "!Failed to query tbl_1 because it does not exist.";
  }

  // Add a field to an existing table
  static DatabaseObject addFieldTBL(std::string db_name, std::string tbl_name, std::string fieldName, std::string fieldType, std::string fieldCount = 1)
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

  // Delete a database by recursively deleting all tables then deleting the directory
  static bool deleteDB(std::string db_name)
  {
    fs::remove_all(DATA_PATH / db_name);
    return !fs::remove(DATA_PATH / db_name);
  }

  // Delete a single table from a database by removing its .scheme file
  // TODO: Delete all its data as well (.data file)
  static bool deleteTBL(std::string db_name, std::string tbl_name)
  {
    return !fs::remove(DATA_PATH / db_name / (tbl_name + ".schema"));
  }

  // Load the a database and all its schemas and data into memory
  static DatabaseObject loadDB(std::string db_name)
  {
    DatabaseObject res(db_name);
    auto db_path = (DATA_PATH / db_name);
    std::string databaseName = "";
    std::string tableName = "";
    for (const auto &file : fs::directory_iterator(db_path))
    {
      auto proto_path = file.path();
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
    return res;
  }
};

#endif /* __PROTO_GENERATOR__ */
