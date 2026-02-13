#include "./launch.h"

#include <utility>

RunTask::RunTask(std::function<void()> run, uint8_t priority)
    : run(std::move(run)), Task<std::monostate>(std::nullopt, priority) {}

void RunTask::execute() { run(); }

std::string RunTask::toString() const {
  return "RunTask(priority=" + std::to_string(priority) + ")";
}

void RunTask::cleanup() {}
