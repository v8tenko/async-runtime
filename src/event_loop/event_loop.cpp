#include <iostream>
#include <thread>

#include "event_loop.h"
#include "task/task.h"

using std::shared_ptr;

void EventLoop::initialize(uint8_t poolSize) {
  eventLoopThread = std::thread(&EventLoop::tick, this);

  pool.initialize(poolSize);
  pool.onComplete([this](shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(resultMutex);
    result.push(std::move(task));
    tickCV.notify_one();
  });
}

void EventLoop::run() {
  std::unique_lock<std::mutex> lock(workDoneMutex);
  workDoneCV.wait(lock, [this] { return pending.load() == 0; });
}

void EventLoop::tick() {
  while (running) {
    if (tryToStartTask()) {
      continue;
    }

    if (tryToFinishTask()) {
      continue;
    }

    std::unique_lock<std::mutex> lock(tickMutex);
    tickCV.wait_for(lock, std::chrono::milliseconds(100));
  }
}


void EventLoop::push(std::shared_ptr<Task> task) {
  std::lock_guard<std::mutex> lock(queueMutex);

  pending.fetch_add(1);
  queue.push(std::move(task));
  tickCV.notify_one();
}

bool EventLoop::tryToStartTask() {
  std::unique_lock<std::mutex> lock(queueMutex);
  if (queue.empty()) {
    return false;
  }

  std::shared_ptr<Task> task = queue.top();
  queue.pop();

  lock.unlock();

  startTask(task);

  return true;
}

bool EventLoop::tryToFinishTask() {
  std::unique_lock<std::mutex> lock(resultMutex);
  if (result.empty()) {
    return false;
  }

  std::shared_ptr<Task> task = result.top();
  result.pop();

  finishTask(task);

  return true;
}

void EventLoop::startTask(std::shared_ptr<Task> &task) {
  pool.schedule(task);
}

void EventLoop::finishTask(std::shared_ptr<Task> &task) {
  task->complete();
  int old = pending.fetch_sub(1);

  if (old == 1) {
    workDoneCV.notify_one();
  }
}

void EventLoop::terminate() {
  running = false;
  
  workDoneCV.notify_one();
  tickCV.notify_one();

  if (eventLoopThread.joinable()) {
    eventLoopThread.join();
  }

  pool.terminate();
}

EventLoop::~EventLoop() {
  terminate();
}