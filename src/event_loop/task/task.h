#pragma once

#include <any>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

enum class TaskState { PENDING, RUNNING, COMPLETED, FAILED, CANCELLED };

template <typename Result = std::any>
using Callback =
    std::function<void(std::optional<std::string>, std::optional<Result>)>;

struct TaskControl {
  std::atomic<bool> cancelRequested = false;
  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;
};

class TaskHandle {
public:
  TaskHandle() = default;
  explicit TaskHandle(std::shared_ptr<TaskControl> control)
      : control(std::move(control)) {}

  void cancel() const {
    if (!control) {
      return;
    }

    control->cancelRequested.store(true);
  }

  void wait() const {
    if (!control) {
      return;
    }

    std::unique_lock<std::mutex> lock(control->mutex);
    control->cv.wait(lock, [this] { return control->done; });
  }

  bool isDone() const {
    if (!control) {
      return true;
    }

    std::lock_guard<std::mutex> lock(control->mutex);
    return control->done;
  }

private:
  std::shared_ptr<TaskControl> control;
};

class BaseTask {
public:
  BaseTask(uint8_t priority) : priority(priority) {}
  virtual ~BaseTask() = default;

  TaskState state = TaskState::PENDING;
  uint8_t priority;

  TaskHandle handle() const { return TaskHandle(control); }

  void requestCancel() { control->cancelRequested.store(true); }

  bool isCancelRequested() const { return control->cancelRequested.load(); }

  virtual void execute() = 0;
  virtual void stop() { throw std::runtime_error("Task aborted"); };
  virtual void cleanup() = 0;

  virtual std::string toString() const = 0;

protected:
  std::shared_ptr<TaskControl> control = std::make_shared<TaskControl>();
  std::optional<std::string> error;

  void markCancelled() {
    error = std::make_optional<std::string>("Cancelled");
    state = TaskState::CANCELLED;
  }

  void markDone() {
    std::lock_guard<std::mutex> lock(control->mutex);
    control->done = true;
    control->cv.notify_all();
  }

  virtual void onError(const std::string &error) = 0;
  virtual void onComplete() = 0;

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

  void onComplete() override { state = TaskState::COMPLETED; }

  void onError(const std::string &_error) override {
    error = std::make_optional<std::string>(_error);
    state = TaskState::FAILED;
  }

  void finish() override {
    if (!callback.has_value()) {
      return;
    }

    callback.value()(error, value);
  }
};
