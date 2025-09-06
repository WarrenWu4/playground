#pragma once

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

// additional performance boosts

class Logger4 {
private:
  const int BUFFER_SIZE = 8 * 1024; // 8KB buffer size
  std::string filename;
  std::fstream file;
  std::size_t maxFileSize;
  std::vector<char> buffer;
  std::size_t bufferSize;
  std::vector<char> temp;
public:
  Logger4(const std::string &filename, std::size_t maxFileSize) {
    file.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    file.close();
    file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
      throw std::runtime_error("Constructor: cannot open file: data.log");
    }
    this->filename = filename;
    this->maxFileSize = maxFileSize;
    buffer.resize(BUFFER_SIZE);
    bufferSize = 0;
    temp.resize(maxFileSize);
  }
  ~Logger4() {
    if (file.is_open()) {
      flush();
      file.close();
    }
  }
  void write(const std::string &data) {
    std::size_t dataSize = data.size();
    if (static_cast<int>(bufferSize + dataSize) > BUFFER_SIZE) {
      flush();
    }
    std::memcpy(buffer.data() + bufferSize, data.data(), dataSize);
    bufferSize += dataSize;
  }
  void flush() {
    int currFileSize = file.tellg();
    if (bufferSize + currFileSize > maxFileSize) {
      int bytesToRemove = bufferSize + currFileSize - maxFileSize;
      int oldDataSize = currFileSize - bytesToRemove;
      file.seekg(bytesToRemove, std::ios::beg);
      file.read(temp.data(), oldDataSize);
      file.seekp(0, std::ios::beg);
      file.write(temp.data(), oldDataSize);
    }
    file.write(buffer.data(), bufferSize);
    bufferSize = 0;
  }
};
