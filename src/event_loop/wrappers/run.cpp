#include <memory>

#include "../task/run/run.h"
#include "./async.h"

void async::run(const std::function<void ()> cb) {
  async::detail::prelude();

	std::shared_ptr<RunTask> task = std::make_shared<RunTask>(std::move(cb));

	EventLoop::instance().push(std::move(task));
}