#include <thread>

#include "event_loop.h"
#include "task/task.h"

using std::shared_ptr;

void EventLoop::start(uint8_t poolSize) {
  eventLoopThread = std::thread(&EventLoop::run, this);

  pool.initialize(poolSize);
  pool.onComplete([this](shared_ptr<Task> task) {
    resultMutex.lock();
    result.emplace(task->priority, std::move(task));
    resultMutex.unlock();
  });
}

void EventLoop::run() {
  while (running) {
    queueMutex.lock();
    if (queue.size()) {

      auto it = queue.begin();
      std::shared_ptr<Task> task = std::move(it->second);
      queue.erase(it);
      queueMutex.unlock();

      pool.queue(task);

      continue;
    } else {
      queueMutex.unlock();
    }

    resultMutex.lock();
    if (result.size()) {
      auto it = result.begin();
      std::shared_ptr<Task> task = std::move(it->second);
      result.erase(it);
      resultMutex.unlock();

      task->complete();
    } else {
      resultMutex.unlock();
    }
  }
}

void EventLoop::push(std::shared_ptr<Task> task) {
  queueMutex.lock();
  queue.emplace(task->priority, std::move(task));
  queueMutex.unlock();
}

void EventLoop::terminate() {
  running = false;
  if (eventLoopThread.joinable()) {
    eventLoopThread.join();
  }
}