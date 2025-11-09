#pragma once

#include <memory>
#include <thread>

#include "task/task.h"
#include "threads_pool/threads_pool.h"

#include "tasks_types.h"

struct EventLoop {
public:
	static EventLoop& instance() {
		static EventLoop instance;

		return instance;
	};

  void push(std::shared_ptr<Task> task);
  void start(uint8_t poolSize = 1);
  void terminate();

  const void operator=(EventLoop &loop) = delete;
  const void operator=(EventLoop &&loop) = delete;

protected:
	EventLoop() = default;

  std::mutex queueMutex;
  TaskQueue queue;
  std::mutex resultMutex;
  TaskQueue result;

  friend class ThreadsPool;

private:
  std::thread eventLoopThread;
  ThreadsPool pool;

  std::atomic<bool> running = true;

  void run();
};