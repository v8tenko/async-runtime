#include <string>
#include <utility>

#include "../task/read_file/read_file_task.h"
#include "./runtime.h"

TaskHandle AsyncRuntime::readFile(std::string path, Callback<std::string> cb) {
  std::unique_ptr<ReadFileTask> task =
      std::make_unique<ReadFileTask>(std::move(path), std::move(cb));

  TaskHandle handle = task->handle();

  loop.push(std::move(task));

  return handle;
}