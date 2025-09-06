#pragma once

#include <fstream>
#include <string>
#include <vector>

// reducing flushes from buffered implementation

class Logger3 {
private:
  const int BUFFER_SIZE = 8 * 1024; // 8KB buffer size
  std::string filename;
  std::fstream file;
  std::size_t maxFileSize;
  std::vector<char> buffer; // in-memory buffer for batching
public:
  Logger3(const std::string &filename, std::size_t maxFileSize) {
    file.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    file.close();
    file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
      throw std::runtime_error("Constructor: cannot open file: data.log");
    }
    this->filename = filename;
    this->maxFileSize = maxFileSize;
  }
  ~Logger3() {
    if (file.is_open()) {
      flush();
      file.close();
    }
  }
  void write(const std::string &data) {
    std::size_t dataSize = data.size();
    // check if buffer is full
    if (static_cast<int>(buffer.size() + dataSize) > BUFFER_SIZE) {
      // flush the buffer
      flush();
    }
    // add new data to buffer
    buffer.insert(buffer.end(), data.begin(), data.end());
  }
  void flush() {
    // check if exceeds file size
    int currFileSize = file.tellg();
    if (buffer.size() + currFileSize > maxFileSize) {
      // if it does, rewrite old data with new data
      int bytesToRemove = buffer.size() + currFileSize - maxFileSize;
      file.seekg(bytesToRemove, std::ios::beg);
      std::vector<char> remainingData(currFileSize - bytesToRemove);
      file.read(remainingData.data(), currFileSize - bytesToRemove);
      file.seekp(0, std::ios::beg);
      file.write(remainingData.data(), remainingData.size());
      file.seekp(remainingData.size(), std::ios::beg);
    }
    file.write(buffer.data(), buffer.size());
    file.seekp(0, std::ios::end); // move to the end after writing
    buffer.clear();               // clear the buffer after writing
  }
};
