#include <memory>

#include "../task/run/run.h"
#include "./runtime.h"

void AsyncRuntime::run(const std::function<void ()> cb) {
	std::shared_ptr<RunTask> task = std::make_shared<RunTask>(std::move(cb));

	loop.push(std::move(task));
}