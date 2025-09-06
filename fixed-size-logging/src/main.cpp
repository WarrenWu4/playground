#include "logger1.h"
#include "logger2.h"
#include "logger3.h"
#include "logger4.h"
#include "test.h"
#include <cstring>

struct Args {
  int numIterations = 100;
  int numMessages = 1000;
  int maxFileSize = 1024; // 1KB
  std::string fileName = "log";
};

void fullBenchmark(Tester t, Args a, std::vector<std::string> &messages) {
  t.benchmark(
      "Naive Implementation",
      [&]() { t.runLogger<Logger1>(Logger1(a.fileName, a.maxFileSize), messages); },
      a.numIterations);
  if (!t.checkAccuracy("log", "comp")) {
    std::cerr << "Logger 1 failed accuracy check\n";
    return;
  }
  t.benchmark(
      "Buffer Implementation",
      [&]() { t.runLogger<Logger2>(Logger2(a.fileName, a.maxFileSize), messages); },
      a.numIterations);
  if (!t.checkAccuracy("log", "comp")) {
    std::cerr << "Logger 2 failed accuracy check\n";
    return;
  }
  t.benchmark(
      "2 Pointer Implementation",
      [&]() { t.runLogger<Logger3>(Logger3(a.fileName, a.maxFileSize), messages); },
      a.numIterations);
  if (!t.checkAccuracy("log", "comp")) {
    std::cerr << "Logger 3 failed accuracy check\n";
    return;
  }
  t.benchmark(
      "Raw Dawg Implementation",
      [&]() { t.runLogger<Logger4>(Logger4(a.fileName, a.maxFileSize), messages); },
      a.numIterations);
  if (!t.checkAccuracy("log", "comp")) {
    std::cerr << "Logger 4 failed accuracy check\n";
    return;
  }
}

int main(int argc, char *argv[]) {
  Tester t;
  Args a;

  if (argc < 3) {
    std::cerr << "Requires num messages and max file size\n";
    return 1;
  }
  a.numMessages = atoi(argv[1]);
  a.maxFileSize = 1024 * atoi(argv[2]);

  std::vector<std::string> messages;
  t.benchmark("Generating Messages", [&]() {
    messages = t.generateMessage(a.numMessages);
  }, 1);

  t.benchmark("Generating File Comp", [&]() {
    Logger3 logger = Logger3("comp", a.maxFileSize);
    for (std::size_t i = 0; i < messages.size(); i++) {
      logger.write(messages.at(i));
    }
  }, 1);

  fullBenchmark(t, a, messages);

  return 0;
}
