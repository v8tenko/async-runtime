#pragma once

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace runtime_test {

struct TempFile {
  std::filesystem::path path;

  explicit TempFile(std::string content) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    path = std::filesystem::temp_directory_path() /
           ("async_runtime_test_" + std::to_string(now) + ".txt");

    std::ofstream out(path);
    out << content;
    out.close();
  }

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

template <class Pred>
bool wait_for(std::condition_variable &cv, std::mutex &mutex,
              std::chrono::milliseconds timeout, Pred pred) {
  std::unique_lock<std::mutex> lock(mutex);
  return cv.wait_for(lock, timeout, pred);
}

} // namespace runtime_test
