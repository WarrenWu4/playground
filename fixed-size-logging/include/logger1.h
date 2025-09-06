#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// naive implementation for fixed size logging

class Logger1 {
private:
  std::fstream file;
  std::string filename;
  std::size_t maxSize;

public:
  Logger1(const std::string &filename, std::size_t maxSize) {
    file.open(filename, std::ios::binary | std::ios::out);
    file.close();
    file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
      throw std::runtime_error("Constructor: cannot open file: " + filename);
    }
    // set variables to correct values
    this->filename = filename;
    this->maxSize = maxSize;
  }
  ~Logger1() {
    if (file.is_open()) {
      file.close();
    }
  }
  void write(const std::string &data) {
    // get size of data and file
    std::size_t dataSize = data.size();
    std::size_t currentSize = file.tellg();
    // check if you need to overrwrite the file
    if (currentSize + dataSize > maxSize) {
      // calculate how many bytes to remove
      std::size_t bytes = (currentSize + dataSize) - maxSize;
      // navigate to the non-removed part of the file
      file.seekg(bytes, std::ios::beg);
      // read remaining data into a buffer
      std::vector<char> buffer(currentSize - bytes);
      file.read(buffer.data(), currentSize - bytes);
      // close the file and reopen it in truncate mode
      file.close();
      file.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
      file.seekp(0, std::ios::beg);
      file.write(buffer.data(), buffer.size());
      // close the file and reopen in append mode
      file.close();
      file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
      file.seekp(0, std::ios::end);
    }
    file.write(data.data(), dataSize);
  }
};
