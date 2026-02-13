#pragma once

#include <condition_variable>
#include <memory>
#include <thread>

#include "task/task.h"
#include "threads_pool/threads_pool.h"

#include "tasks_types.h"

class EventLoop {
public:
  void initialize(uint8_t poolSize = 1);
  void run();
  void push(std::unique_ptr<BaseTask> task);
  void terminate();

  std::thread::id threadId() const { return loopThreadId; }

  void operator=(EventLoop &loop) = delete;
  void operator=(EventLoop &&loop) = delete;

  ~EventLoop();

protected:
  std::mutex queueMutex;
  TaskQueue queue;
  std::mutex resultMutex;
  TaskQueue result;

  friend class ThreadsPool;

private:
  std::thread eventLoopThread;
  ThreadsPool pool;

  std::thread::id loopThreadId;
  std::mutex loopThreadIdMutex;
  std::condition_variable loopThreadIdCV;
  bool loopThreadIdReady = false;

  std::mutex workDoneMutex;
  std::condition_variable workDoneCV;
  std::atomic<int> pending = 0;
  void startTask(std::unique_ptr<BaseTask> task);
  void finishTask(std::unique_ptr<BaseTask> task);

  std::atomic<bool> running = true;
  std::atomic<bool> stopping = false;

  void tick();
  bool tryToStartTask();
  bool tryToFinishTask();

  std::mutex tickMutex;
  std::condition_variable tickCV;
};