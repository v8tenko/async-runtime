#include "./run.h"

RunTask::RunTask(std::function<void()> cb, uint8_t priority)
    : cb(std::move(cb)), Task(priority) {}

void RunTask::execute() { cb(); }

std::string RunTask::toString() const {
  return "RunTask(priority=" + std::to_string(priority) + ")";
}

void RunTask::cleanup() {}
void RunTask::complete() {}