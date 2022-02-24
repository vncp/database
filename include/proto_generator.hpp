#ifndef __PROTO_GENERATOR__
#define __PROTO_GENERATOR__

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>
#include <data_objs.hpp>

namespace fs = std::experimental::filesystem;

namespace paths {
  const std::string PROTO_VERSION = "syntax = \"proto3\";";
  auto PROTOC_PATH = fs::current_path() / "etc" / "protobuf" / "src" / "protoc";
  auto PROJECT_ROOT = fs::current_path();
  auto DATA_PATH = fs::current_path() / "data";
};
using namespace paths;

class ProtoGenerator {
  DatabaseObject db_obj;

  void verifyProtoc() {
    if (!fs::exists(PROTOC_PATH)) {
      std::cout << "This project depends on Google Protocol Buffers to store data as bytes.\n" <<
               "The application will now download, build, and install the protobuf\n" <<
               "compiler and proto-cpp in the local application directory (~20 mins).\n" <<
               "CTRL-C to exit or Enter to accept.\n";
      int a;
      cin >> a;
      system((PROJECT_ROOT / "install_protoc.sh").c_str());
    } else {
      std::cout << "Found protoc binary at " << PROTOC_PATH << endl;
    }
    if (!fs::exists(PROTOC_PATH)) {
      cerr << "Could not install protoc." << endl;
    }
    // Make sure out directory for protoc exists
    fs::create_directories(PROJECT_ROOT / "include" / "generated");
  }

  // Create a new Table
  void protocGenerate(DatabaseObject database, TableObject table) {
    if (!fs::exists(DATA_PATH / database.name())) {
      fs::create_directories(DATA_PATH / database.name());
    }
    std::ofstream protoFile(DATA_PATH / database.name());
    protoFile << PROTO_VERSION << "\n";
    protoFile << "package " << database.name() << ";\n";
    protoFile << generateMetadataComment(table.maxFieldNum, table.name(), database.name()) << "\n";
    protoFile << "message " << table.name() << " {\n";
    for (auto field : table.fields) {
      protoFile << "\t" << get<0>(field.second) << " " << field.first << " = " << get<2>(field.second) << ";\n";
    }
    protoFile << "}\n";
    protoFile.flush();
    protoFile.close();
  }
  
  static std::string generateMetadataComment(const int maxNumField,
                                             std::string_view tableName,
                                             std::string_view databaseName) {
    std::ostringstream ss;
    ss << "/*    METADATA-START\n";
    ss << "maxFieldNum " << maxNumField << std::endl;
    ss << "tableName " << tableName << std::endl; 
    ss << "databaseName " << databaseName << std::endl;
    ss << "METADATA-END    */\n";
    return ss.str();
  }

public:
  ProtoGenerator(DatabaseObject db_obj) : db_obj(db_obj) {
    verifyProtoc();
    auto dbNamePath = DATA_PATH / db_obj.name();
    if (!fs::exists(dbNamePath)) {
      fs::create_directories(dbNamePath);
    }
    for (const auto &table : db_obj.tables) {
      // If there's not a file for a table, create the table
      if (!fs::exists(dbNamePath / (std::string(table.name()) + ".proto"))){
        protocGenerate(db_obj, table);
      }
    }
  }



};

#endif /* __PROTO_GENERATOR__ */