#pragma once

#include <memory>
#include <thread>

#include "task/task.h"
#include "threads_pool/threads_pool.h"

#include "tasks_types.h"

class EventLoop {
public:
	static EventLoop& instance() {
		static EventLoop instance;

		return instance;
	};

  void initialize(uint8_t poolSize = 1);
  void run();
  void push(std::shared_ptr<Task> task);
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

  std::mutex workDoneMutex;
  std::condition_variable workDoneCV;
  std::atomic<int> pending = 0;
  void startTask(std::shared_ptr<Task> &task);
  void finishTask(std::shared_ptr<Task> &task);

  std::atomic<bool> running = true;

  void tick();
  bool tryToStartTask();
  bool tryToFinishTask();

  std::mutex tickMutex;
  std::condition_variable tickCV;
};