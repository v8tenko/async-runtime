#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "../task/task.h"
#include "observable/observable.h"

class ThreadsPool {
public:
  void initialize(uint8_t count);

  void schedule(std::shared_ptr<BaseTask> task);
  Observable<std::shared_ptr<BaseTask>> $complete;

  void terminate();

private:
  std::atomic<bool> running = true;

  std::vector<std::thread> threads;

	void runTask(std::shared_ptr<BaseTask> task);

  std::mutex mutex;
  std::deque<std::shared_ptr<BaseTask>> queue;

  std::condition_variable newTaskCV;

  void tick();
};