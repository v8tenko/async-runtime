#include "threads_pool.h"

void ThreadsPool::initialize(uint8_t count) {
  threads = std::vector<std::thread>();
  threads.reserve(count);

  for (uint8_t i = 0; i < count; i++) {
    threads.emplace_back(&ThreadsPool::run, this);
  }
}

void ThreadsPool::queue(std::shared_ptr<Task> task) {
	mutex.lock();
	_queue.push_back(task);
	mutex.unlock();
}

void ThreadsPool::run() {
		while (running) {
			mutex.lock();
			if (_queue.empty()) {
				mutex.unlock();
				
				continue;
			}
			std::shared_ptr<Task> task = _queue.front();
			_queue.pop_front();
			mutex.unlock();

			task->execute();

			for (auto& handler: handlers) {
				handler(task);
			}
		}
}

void ThreadsPool::onComplete(TaskHandler handler) {
	handlers.push_back(std::move(handler));
}

void ThreadsPool::terminate() {
  running = false;

  for (auto &thread : threads) {
    if (thread.joinable()) {
			thread.join();
		}
  }
}