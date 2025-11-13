#include "threads_pool.h"

void ThreadsPool::initialize(uint8_t count) {
  threads = std::vector<std::thread>();
  threads.reserve(count);

  for (uint8_t i = 0; i < count; i++) {
    threads.emplace_back(&ThreadsPool::tick, this);
  }
}

void ThreadsPool::schedule(std::shared_ptr<BaseTask> task) {
  std::lock_guard<std::mutex> lock(mutex);
  queue.push_back(task);
  newTaskCV.notify_one();
}

void ThreadsPool::tick() {
  while (running) {
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
      newTaskCV.wait_for(lock, std::chrono::milliseconds(100));

      continue;
    }

    std::shared_ptr<BaseTask> task = queue.front();
    queue.pop_front();

    lock.unlock();

    runTask(task);
  }
}

void ThreadsPool::runTask(std::shared_ptr<BaseTask> task) {
  try {
    task->state = TaskState::PENDING;
    task->execute();

    task->state = TaskState::COMPLETED;
  } catch (const std::exception &error) {
    task->onError(error.what());
    task->state = TaskState::FAILED;
  }

  $complete.trigger(task);
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