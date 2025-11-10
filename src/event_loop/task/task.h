#pragma once

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <string>

class Task {
public:
  Task(uint8_t priority) : priority(priority) {}

  virtual ~Task() = default;

  virtual void execute() = 0;

  virtual void abort() { aborted = true; };

  virtual void cleanup() = 0;
  virtual std::string toString() const = 0;

  uint8_t priority;

protected:
  std::atomic<bool> aborted;

  virtual void complete() = 0;

  void abortTask() { throw std::runtime_error("Task aborted"); }

  friend class EventLoop;
};

inline std::ostream &operator<<(std::ostream &stream, const Task &task) {
  return stream << task.toString();
}
