#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

class Timer {
private:
  std::chrono::high_resolution_clock::time_point start_time;

public:
  void start() { start_time = std::chrono::high_resolution_clock::now(); }

  double elapsed_ms() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    return duration.count() / 1000.0; // milliseconds
  }
};

class Tester {
public:
  std::vector<std::string> generateMessage(int numMessages = 1, int minLen = 1,
                                           int maxLen = 1000) {
    static const std::string charset = "abcdefghijklmnopqrstuvwxyz"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "0123456789";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> lengthDist(minLen, maxLen);
    std::uniform_int_distribution<int> charDist(0, charset.size() - 1);
    std::vector<std::string> results;
    results.reserve(numMessages);
    for (int i = 0; i < numMessages; i++) {
      int length = lengthDist(rng);
      std::string str;
      str.reserve(length);
      for (int j = 0; j < length-1; j++) {
        str.push_back(charset[charDist(rng)]);
      }
      str.push_back('\n');
      results.push_back(std::move(str));
    }
    return results;
  }

  template <typename T>
  void runLogger(T obj, std::vector<std::string> &messages) {
    for (size_t i = 0; i < messages.size(); i++) {
      obj.write(messages.at(i));
    }
  }

  template <typename Func, typename Setup = std::function<void()>>
  double benchmark(const std::string &name, Func func, int iterations = 1) {
    Timer timer;
    std::cout << "============================================\n";
    std::cout << "Testing: " << name << std::flush;
    timer.start();
    for (int i = 0; i < iterations; i++) {
      func();
    }
    double elapsed = timer.elapsed_ms();
    std::cout << "\n - " << std::fixed << std::setprecision(3) << elapsed
              << " ms";
    if (iterations > 1) {
      std::cout << " (avg: " << elapsed / iterations << " ms per iteration)";
    }
    std::cout << std::endl;
    std::cout << "============================================\n";
    return elapsed;
  }

  bool checkAccuracy(const std::string &_f1, const std::string &_f2) {
    std::ifstream f1(_f1);
    std::ifstream f2(_f2);
    if (!f1 || !f2) {
      std::cerr
          << "Failed to open one or both files while checking accuracy.\n";
      std::cout << " - Accuracy: FAIL\n";
      return false;
    }
    std::ostringstream ss1, ss2;
    ss1 << f1.rdbuf();
    ss2 << f2.rdbuf();
    if (ss1.str() == ss2.str()) {
      std::cout << " - Accuracy: PASS\n";
      return true;
    } else {
      std::cout << " - Accuracy: FAIL\n";
      return false;
    }
  }
};
