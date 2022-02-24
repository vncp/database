#ifndef __PROTO_GENERATOR__
#define __PROTO_GENERATOR__

#include <iostream>
#include <string>
#include <string_view>
#include <fstream>
#include <experimental/filesystem>
#include <data_objs.hpp>

namespace fs = std::experimental::filesystem;

const std::string PROTO_VERSION = "syntax = \"proto3\";";

class ProtoGenerator {
  size_t max_field_num = 1;

public:
  ProtoGenerator(std::string database_name, std::string table_name) {
    auto path = fs::current_path();
    path += "/" + database_name;
    if (!fs::is_directory(path)) {
      fs::create_directory(path);
    }
    path += "/" + table_name + ".proto";
    // If the path exists then read the max (last) field number
    int maxFieldCandidate = 1;
    std::string prefix = "";
    if (fs::exists(path)) {
      std::ifstream proto_file_in(path);
      while (prefix != "MaxFieldNum") {
        proto_file_in >> prefix;
      }
      proto_file_in >> maxFieldCandidate;
      proto_file_in.close();
    } else { // Generate file if it doesn't exist
      std::ofstream proto_file_out(path);
      proto_file_out << "/* META DATA, DO NOT DELETE\n\tMaxFieldNum " << max_field_num << "\n*/\n";
      proto_file_out << PROTO_VERSION;
      proto_file_out.flush();
      proto_file_out.close();
    }
  }

};

#endif /* __PROTO_GENERATOR__ */