#pragma once

#include <any>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>

class Json {
private:
  std::unordered_map<std::string, std::any> data;


  void removeAllWhitespaces(std::string& str) {
    size_t writePtr = 0;
    for (size_t i = 0; i < str.size(); i++) {
      if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') { continue; } 
      str[writePtr] = str[i];
      writePtr += 1;
    }
    for (size_t i = writePtr; i < str.size(); i++) {
      str[i] = ' ';
    }
  }

  void parseData(std::string_view processedData) {
    std::regex integerType("^[0-9]+$");
    std::regex doubleType("^[0-9]+(\\.[0-9]+)?$");
    std::string key = "";
    std::string value = "";
    bool isKey = true;
    for (size_t i = 0; i < processedData.size(); i++) {
      if (processedData[i] == '"') { continue; }
      if (processedData[i] == '{' || processedData[i] == '}') {
        if (key != "" && value != "") {
          data[key] = value;
        }
      } else if (processedData[i] == ',') {
        isKey = true;
        if (std::regex_match(value, integerType)) {
          data[key] = std::stoi(value);
        } else if (std::regex_match(value, doubleType)) {
          data[key] = std::stod(value);
        } else {
          data[key] = value;
        }
        key = "";
        value = "";
      } else if (processedData[i] == ':') {
        isKey = false;
      } else if (isKey) {
        key += processedData[i];
      } else {
        value += processedData[i];
      }
    }
  }

public:
  Json() {}
  ~Json() {}
  std::unordered_map<std::string, std::any> parse(std::string_view path) {
    std::string rawData;
    readData(path, rawData);
    removeAllWhitespaces(rawData);
    parseData(rawData);
    return data;
  }
  void readData(std::string_view path, std::string &rawData) {
    std::ifstream f;
    f.open(path.data(), std::ios::in);
    if (!f.is_open()) {
      std::cerr << "Unable to open file\n";
      return;
    }
    std::string temp;
    while (std::getline(f, temp)) {
      rawData += temp;
    }
    f.close();
  }
};
