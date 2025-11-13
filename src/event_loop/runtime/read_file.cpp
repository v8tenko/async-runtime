#include <string>

#include "../task/read_file/read_file_task.h"
#include "./runtime.h"

void AsyncRuntime::readFile(std::string path, Callback<std::string> cb) {
  std::shared_ptr<ReadFileTask> task =
      std::make_shared<ReadFileTask>(std::move(path), std::move(cb));

  loop.push(std::move(task));
}