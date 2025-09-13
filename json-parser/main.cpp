#include "json.h"
#include "jsonge.h"
#include <exception>
#include <string>

int main() {
  Json j = Json();
  std::string path = "test.json";
  std::string data;
  j.readData(path, data);
  try {
    std::cout << "Starting\n";
    json::Value root = json::parse(data);
    std::cout << root["test"].asInt() << "\n";
    std::cout << json::stringify(root) << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
  return 0;
}
