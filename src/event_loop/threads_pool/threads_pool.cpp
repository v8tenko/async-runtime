#include "threads_pool.h"

#include <chrono>
#include <utility>

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
  while (true) {
    std::unique_lock<std::mutex> lock(mutex);

    newTaskCV.wait_for(lock, std::chrono::milliseconds(100),
                       [this] { return !queue.empty() || !running.load(); });

    if (!running.load() && queue.empty()) {
      return;
    }

    if (queue.empty()) {
      continue;
    }

    std::unique_ptr<BaseTask> task = std::move(queue.front());
    queue.pop_front();

    lock.unlock();

    runTask(std::move(task));
  }
}

void ThreadsPool::runTask(std::unique_ptr<BaseTask> task) {
  try {
    if (task->isCancelRequested()) {
      task->markCancelled();
    } else {
      task->state = TaskState::RUNNING;
      task->execute();
      task->onComplete();
    }
  } catch (const std::exception &error) {
    task->onError(error.what());
  } catch (...) {
    task->onError("Unknown error");
  }

  $complete.trigger(std::move(task));
}

void ThreadsPool::requestStop() {
  running = false;
  newTaskCV.notify_all();
}

std::deque<std::unique_ptr<BaseTask>> ThreadsPool::stealQueued() {
  std::lock_guard<std::mutex> lock(mutex);
  std::deque<std::unique_ptr<BaseTask>> out;
  out.swap(queue);
  return out;
}

void ThreadsPool::join() {
  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void ThreadsPool::terminate() {
  requestStop();
  join();
}