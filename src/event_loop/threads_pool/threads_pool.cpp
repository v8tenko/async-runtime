#include "threads_pool.h"

void ThreadsPool::initialize(uint8_t count) {
  threads = std::vector<std::thread>();
  threads.reserve(count);

  for (uint8_t i = 0; i < count; i++) {
    threads.emplace_back(&ThreadsPool::run, this);
  }
}

void ThreadsPool::schedule(std::shared_ptr<Task> task) {
	std::lock_guard<std::mutex> lock(mutex);
	queue.push_back(task);
	newTaskCV.notify_one();
}

void ThreadsPool::run() {
		while (running) {
			std::unique_lock<std::mutex> lock(mutex);
			if (queue.empty()) {
				newTaskCV.wait_for(lock, std::chrono::milliseconds(100));

				continue;
			}

			std::shared_ptr<Task> task = queue.front();
			queue.pop_front();
			
			lock.unlock();

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

	newTaskCV.notify_all();

  for (auto &thread : threads) {
    if (thread.joinable()) {
			thread.join();
		}
  }
}