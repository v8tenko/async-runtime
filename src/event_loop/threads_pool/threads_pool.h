#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <thread>
#include <vector>

#include "../task/task.h"
#include "observable/observable.h"

class ThreadsPool {
public:
  void initialize(uint8_t count);

  void schedule(std::unique_ptr<BaseTask> task);
  Observable<std::unique_ptr<BaseTask>> $complete;

  void requestStop();
  std::deque<std::unique_ptr<BaseTask>> stealQueued();
  void join();
  void terminate();

private:
  std::atomic<bool> running = true;

  std::vector<std::thread> threads;

  void runTask(std::unique_ptr<BaseTask> task);

  std::mutex mutex;
  std::deque<std::unique_ptr<BaseTask>> queue;

  std::condition_variable newTaskCV;

  void tick();
};