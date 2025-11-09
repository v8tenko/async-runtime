#include <string>

#include "../task/read_file/read_file_task.h"
#include "./async.h"

void async::readFile(std::string path, std::function<void(std::string)> cb) {
  async::detail::prelude();

  std::shared_ptr<ReadFileTask> task =
      std::make_shared<ReadFileTask>(std::move(path), std::move(cb));

  EventLoop::instance().push(std::move(task));
}