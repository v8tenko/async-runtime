#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

#include "../task.h"
#include "read_file_task.h"

ReadFileTask::ReadFileTask(std::string path,
                           std::function<void(std::string)> cb,
                           uint8_t priority)
    : path(std::move(path)), cb(cb), Task(priority) {}

void ReadFileTask::execute() {
  file = std::ifstream(path);

  std::ostringstream buffer;
  std::string line;

  while (std::getline(file, line)) {
    if (aborted) {
      abortTask();
    }

    buffer << line << '\n';
  }

  result = std::make_unique<std::string>(buffer.str());
}

void ReadFileTask::cleanup() { file.close(); }

void ReadFileTask::complete() { cb(*result); }

std::string ReadFileTask::toString() const {
  return "ReadFileTask(path=\"" + path + "\", priority=\"" +
         std::to_string(priority) + "\")";
};