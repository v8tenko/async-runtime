#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

enum class TaskState { PENDING, RUNNING, COMPLETED, FAILED };

template <typename Result = std::any>
using Callback = std::function<void(std::optional<std::string>, std::optional<Result>)>;

class BaseTask {
public:
  BaseTask(uint8_t priority) : priority(priority) {}
  ~BaseTask() = default;

  TaskState state = TaskState::PENDING;
  uint8_t priority;

  virtual void execute() = 0;
  virtual void stop() { throw std::runtime_error("Task aborted"); };
  virtual void cleanup() = 0;

  virtual std::string toString() const = 0;

protected:
  std::optional<std::string> error;
  virtual void onError(const std::string& error) = 0;

  virtual void finish() = 0;

  friend class EventLoop;
  friend class ThreadsPool;
};

template <typename Result> class Task : public BaseTask {
public:
  Task(std::optional<Callback<Result>> callback, uint8_t priority)
      : callback(std::move(callback)), BaseTask(priority) {};

protected:
  std::optional<Callback<Result>> callback;
  std::optional<Result> value;
  
  void onError(const std::string& _error) override {
    error = std::make_optional<std::string>(_error);
  }

  void finish() override {
    if (!callback.has_value()) {
      return;
    }

    callback.value()(error, value);
  }
};
