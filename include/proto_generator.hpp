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
using fieldmapType = std::vector<std::pair<std::string, std::tuple<std::string, int>>>;

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
    // Write metadata and fields
    protoFile << generateMetadataComment(table.name(), database->name());
    protoFile << "message " << table.name() << " {\n";
    for (auto field : table.fields)
    {
      protoFile << "\t" << field.first << " = " << get<0>(field.second) << " " << get<1>(field.second) << "\n";
    }
    protoFile << "}\n\n";

    // Write data
    for (auto record : table.records) {
      if (auto val = std::get_if<std::string>(&record))
      {
        protoFile << "'" << *val << "'";
      }
      else if (auto val = std::get_if<int>(&record))
      {
        protoFile << *val;
      }
      else if (auto val = std::get_if<double>(&record))
      {
        protoFile << *val;
      }
      protoFile << "\n";
    }

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

  /**
   * @brief Get rows to print
   * 
   * @param tbl The table to look through
   * @param where the filter query for where expr
   * @return std::vector<int> 
   */
  static std::vector<int> getWhereRows(TableObject tbl,
                                       std::tuple<std::string, std::string, std::string> *where = nullptr)
  {
    // Find Column Described by where[0]
    auto record_iter = tbl.records.begin();
    int row = 0;
    int col = 0;
    int whereCol = -1;
    for (auto rec = tbl.fields.begin(); rec != tbl.fields.end(); rec++) {
      if (where != nullptr && rec->first == get<0>(*where)) {
        whereCol = col;
      }
    }

    // Filter out rows described by where
    std::vector<int> acceptedRows;
    int rows = tbl.records.size() / tbl.fields_size;
    acceptedRows.reserve(rows);
    int cols = tbl.fields_size;
    record_iter = tbl.records.begin();
    for (row = 0; row < rows; row++) {
      // Accept all columns
      if (whereCol == -1) {
        acceptedRows.push_back(row);
      } else {
        auto begin_iter = record_iter;
        // Test String to compare to
        std::string test = std::get<2>(*where);
        // Check if whereCol constraints
        // Make appropriate casts for each type and operator
        // Try catches are for when user inputs different type than expected for where expr
        bool rowCheck = false;
        for (col = 0; col < cols; col++) {
          if (whereCol == col) {
            if (std::get<1>(*where) == "=") {
              if (auto val = std::get_if<std::string>(&(*record_iter))) {
                if (*val == test) rowCheck = true;
              } else if (auto val = std::get_if<int>(&(*record_iter))) {
                try {
                  if (*val == std::stoi(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              } else if (auto val = std::get_if<double>(&(*record_iter))) {
                try {
                  if (*val == std::stod(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              }
            } else if (std::get<1>(*where) == "!=") {
              if (auto val = std::get_if<std::string>(&(*record_iter))) {
                if (*val != test) rowCheck = true;
              } else if (auto val = std::get_if<int>(&(*record_iter))) {
                try {
                  if (*val != std::stoi(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              } else if (auto val = std::get_if<double>(&(*record_iter))) {
                try {
                  if (*val != std::stod(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              }
            } else if (std::get<1>(*where) == "<") {
              if (auto val = std::get_if<std::string>(&(*record_iter))) {
                if (*val < test) rowCheck = true;
              } else if (auto val = std::get_if<int>(&(*record_iter))) {
                try {
                  if (*val < std::stoi(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              } else if (auto val = std::get_if<double>(&(*record_iter))) {
                try {
                  if (*val < std::stod(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              }
            } else if (std::get<1>(*where) == ">") {
              if (auto val = std::get_if<std::string>(&(*record_iter))) {
                if (*val > test) rowCheck = true;
              } else if (auto val = std::get_if<int>(&(*record_iter))) {
                try {
                  if (*val > std::stoi(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              } else if (auto val = std::get_if<double>(&(*record_iter))) {
                try {
                  if (*val > std::stod(test)) rowCheck = true;
                } catch (const std::invalid_argument &ia) {}
              }
            }
          }
          record_iter++;
        }
        // We found a condition that matched so push to acceptedRows
        if (rowCheck) {
          acceptedRows.push_back(row);
        }
      }
    }
    return acceptedRows;
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
    if (!fs::exists(DATA_PATH))
    {
      fs::create_directories(DATA_PATH);
    }
    if (fs::exists(DATA_PATH / db_name))
    {
      return false;
    }
    return fs::create_directory(DATA_PATH / db_name);
  }


  /**
   * @brief Creates a table from fieldmap type, adds to a database, and returns that db object
   * 
   * @param db_name the string of the database name
   * @param tbl_name the string of the table name
   * @param fields the fields defining the table schema
   * @return DatabaseObject the updated database object
   */
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

  static DatabaseObject deleteTBL (std::string db_name,
                                   std::string tbl_name,
                                   std::tuple<std::string, std::string, std::string> * where = nullptr)
  {
    DatabaseObject db = loadDB(db_name);
    for (auto &table : db.tables) {
      if (table.name() == tbl_name) {
        int rows = table.records.size() / table.fields_size;
        int cols = table.fields_size;
        // Get the rows we want to delete (always sorted based on implementation)
        auto rowsToDelete = ProtoGenerator::getWhereRows(table, where);
        // Quick algorithm to delete all the right rows in the organized array
        // Delete from the back so values dont change as we iterate
        for (auto rowToDelete = rowsToDelete.rbegin(); rowToDelete != rowsToDelete.rend(); rowToDelete++) {
          int from = *rowToDelete * cols;
          int to = from + cols;
          table.records.erase(table.records.begin() + from, table.records.begin() + to);
        }
        ProtoGenerator pg(&db);
        return db;
      }
    }
    return DatabaseObject("nil");
  }

  /**
   * @brief prints table records based on a variety of constraints
   * 
   * The fields are scanned to determine the column of the filter and where constraints
   *
   * @param db_name the database name
   * @param tbl_name the table name
   * @param where [default: nullptr]] (column_name operator value) to test for when printing values
   * @return DatabaseObject updated database object
   */
  static DatabaseObject updateTBL (std::string db_name,
                         std::string tbl_name,
                         std::unordered_map<std::string, std::string> what,
                         std::tuple<std::string, std::string, std::string> *where = nullptr)
  {
    DatabaseObject db = loadDB(db_name);
    for (auto &table : db.tables) {
      if (table.name() == tbl_name) {
        int rows = table.records.size() / table.fields_size;
        int cols = table.fields_size;
        // Find all the rows found by query and map to an associative array
        std::vector<bool> acceptedRows(rows, false);
        for(auto row : ProtoGenerator::getWhereRows(table, where)) {
          acceptedRows[row] = true;
        }
        // Go through every row/col and if it's an acceptedRow change its values
        std::string tableFormat = table.getFormat();
        auto record_iter = table.records.begin();
        for (int row = 0; row != rows; row++) {
          for (int col = 0; col != cols; col++) {
            if (acceptedRows[row] && what.find(table.fields[col].first) != what.end()) {
              if (tableFormat[col] == 's') {
                *record_iter = what[table.fields[col].first];
              } else if (tableFormat[col] == 'i') {
                *record_iter = std::stoi(what[table.fields[col].first]);
              } else if (tableFormat[col] == 'f') {
                *record_iter = std::stoi(what[table.fields[col].first]);
              }
            }
            record_iter++;
          }
        }
        // Memory instance updated, now apply to file and update current_database
        ProtoGenerator pg(&db);
        return db;
      }
    }
    return DatabaseObject("nil");
  }

  /**
   * @brief prints table fields and records based on a variety of constraints
   * 
   * The fields are scanned to determine the column of the filter and where constraints
   *
   * @param db_name the database name
   * @param tbl_name the table name
   * @param filter [default: nullptr] vector pointer with column names to print
   * @param where [default: nullptr]] (column_name operator value) to test for when printing values
   * @return std::string
   */
  static std::string printTBL(std::string db_name, 
                              std::string tbl_name, 
                              std::vector<std::string> *filter = nullptr, 
                              std::tuple<std::string, std::string, std::string> *where = nullptr)
  {
    DatabaseObject db = loadDB(db_name);
    for (int i = 0; i < db.tables.size(); i++)
    {
      if (db.tables[i].name() == tbl_name)
      {
        fieldmapType fields = db.tables[i].fields;
        std::ostringstream ss;
        ss << "| ";
        std::vector<bool> skipCols(db.tables[i].fields_size, true);

        // If query contains asterisk or is null, then don't filter
        bool filtered = true;
        if (filter == nullptr) {
          filtered = false;
        } else {
          for (auto query : *filter) {
            if (query == "*") {
              filtered = false;
            }
          }
        }

        int col = 0;
        for (auto i = fields.begin(); i != fields.end(); i++)
        {
          auto name = i->first;
          auto field = i->second;
          if (filter != nullptr) {
            for (auto q : *filter) {
              if (name == q) {
                ss << name << " " << get<0>(field);
                ss << (get<1>(field) > 1 ? ("(" + to_string(get<1>(field)) + ")") : "");
                ss << " | ";
                skipCols[col] = false;
              }
            }
          }
          else // add all
          {
            ss << name << " " << get<0>(field);
            ss << (get<1>(field) > 1 ? ("(" + to_string(get<1>(field)) + ")") : "");
            ss << " | ";
          }


          col++;
        }
        ss << "\n";

        int rows = db.tables[i].records.size() / db.tables[i].fields_size;
        vector<bool> acceptRows(rows, false);
        for(auto row : getWhereRows(db.tables[i], where)) {
          acceptRows[row] = true;
        }
        int cols = db.tables[i].fields_size;
        auto record_iter = db.tables[i].records.begin();
        // Print if it meets constraints
        for (int row = 0; row < rows; row++)
        {
          if (acceptRows[row]) {
            ss << "| ";
          }
          for (int col = 0; col < cols; col++)
          {
            if (acceptRows[row] && (!filtered || !skipCols[col])) {
              if (auto val = std::get_if<std::string>(&(*record_iter)))
              {
                ss << *val << " | ";
              }
              else if (auto val = std::get_if<int>(&(*record_iter)))
              {
                ss << *val << " | ";
              }
              else if (auto val = std::get_if<double>(&(*record_iter)))
              {
                ss << *val << " | ";
              }
            }
            record_iter++;
          }
          ss << "\n";
        }
        return ss.str();
      }
    }
    return "!Failed to query table because it does not exist.";
  }

  // add a field to an existing table
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
      std::string count = "";
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
      while (count != "}")
      {
        while (curr != "=")
        {
          prev = curr;
          curr = next;
          next = count;
          db_file >> count;
        }
        // std::cout << prev << ", " << curr << ',' << next << ", " << count << std::endl;
        tbl.addField(prev, next, stoi(count.c_str()));
        prev = curr;
        curr = next;
        next = count;
        db_file >> count;
      }

      // Now get the format and read in the data
      int size = tbl.fields.size();
      std::string format = tbl.getFormat();

      // Read in row
      //db_file >> count;
      while (!db_file.eof())
      {
        for (int i = 0; i < size; i++)
        {
          db_file >> count;
          if (format[i] == 's')
          {
            std::string fullString = count;
            while (count[count.size() - 2] != '\'')
            {
              db_file >> count;
              fullString += " " + count;
            }

            // std::cout << "Text: " << fullString.substr(1, fullString.size() - 3).c_str() << std::endl;
            char type[1] = {format[i]};
            tbl.addRecord(type, fullString.substr(1, fullString.size() - 3).c_str());
          }
          else if (format[i] == 'f')
          {
            // std::cout << "Number: " << count.substr(0, count.size() - 1) << std::endl;
            char type[1] = {format[i]};
            tbl.addRecord(type, std::stod(&count[0]));
          }
          else if (format[i] == 'i')
          {
            char type[1] = {format[i]};
            tbl.addRecord(type, std::stoi(&count[0]));
          }
        }
      }

      res.insertTable(tbl);
      db_file.close();
    }

    return res;
  }
};

#endif //__PROTO_GENERATOR__