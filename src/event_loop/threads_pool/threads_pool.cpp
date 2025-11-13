#include "threads_pool.h"

void ThreadsPool::initialize(uint8_t count) {
  threads = std::vector<std::thread>();
  threads.reserve(count);

  for (uint8_t i = 0; i < count; i++) {
    threads.emplace_back(&ThreadsPool::tick, this);
  }
}

void ThreadsPool::schedule(std::unique_ptr<BaseTask> task) {
  std::lock_guard<std::mutex> lock(mutex);
  queue.push_back(std::move(task));
  newTaskCV.notify_one();
}

void ThreadsPool::tick() {
  while (running) {
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
      newTaskCV.wait_for(lock, std::chrono::milliseconds(100));

      continue;
    }

    std::unique_ptr<BaseTask> task = std::move(queue.front());
    queue.pop_front();

    lock.unlock();

    runTask(std::move(task));
  }
}

void ThreadsPool::runTask(std::unique_ptr<BaseTask> task) {
  task->state = TaskState::PENDING;

  try {
    task->execute();
    task->onComplete();
  } catch (const std::exception &error) {
    task->onError(error.what());
  }

  $complete.trigger(std::move(task));
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