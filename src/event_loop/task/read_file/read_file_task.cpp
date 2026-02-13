#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

#include "../task.h"
#include "read_file_task.h"

ReadFileTask::ReadFileTask(std::string path, Callback<std::string> cb,
                           uint8_t priority)
    : path(std::move(path)), Task(std::move(cb), priority) {}

void ReadFileTask::execute() {
  file = std::ifstream(path);

  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path);
  }

  std::ostringstream buffer;
  std::string line;

  while (std::getline(file, line)) {
    buffer << line << '\n';
  }

  value = std::optional<std::string>(buffer.str());
}

void ReadFileTask::cleanup() { file.close(); }

std::string ReadFileTask::toString() const {
  return "ReadFileTask(path=\"" + path + "\", priority=\"" +
         std::to_string(priority) + "\")";
};