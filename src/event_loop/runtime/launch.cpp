#include <memory>

#include "../task/launch/launch.h"
#include "./runtime.h"

void AsyncRuntime::launch(const std::function<void()> cb) {
  std::unique_ptr<RunTask> task = std::make_unique<RunTask>(std::move(cb));

  loop.push(std::move(task));
}