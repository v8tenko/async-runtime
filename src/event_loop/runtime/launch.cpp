#include <memory>
#include <utility>

#include "../task/launch/launch.h"
#include "./runtime.h"

TaskHandle AsyncRuntime::launch(std::function<void()> cb) {
  std::unique_ptr<RunTask> task = std::make_unique<RunTask>(std::move(cb));

  TaskHandle handle = task->handle();

  loop.push(std::move(task));

  return handle;
}