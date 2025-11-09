#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "../tasks_types.h"

class ThreadsPool {
public:
  void initialize(uint8_t count);

  void schedule(std::shared_ptr<Task> task);
  void onComplete(TaskHandler handler);

  void terminate();

private:
  std::atomic<bool> running = true;

  std::vector<std::thread> threads;
  std::vector<TaskHandler> handlers;

  std::mutex mutex;
  std::deque<std::shared_ptr<Task>> queue;

  std::condition_variable newTaskCV;

  void run();
};