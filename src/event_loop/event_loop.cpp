#include <thread>
#include <utility>

#include "event_loop.h"
#include "task/task.h"

void EventLoop::initialize(uint8_t poolSize) {
  eventLoopThread = std::thread(&EventLoop::tick, this);

  {
    std::unique_lock<std::mutex> lock(loopThreadIdMutex);
    loopThreadIdCV.wait(lock, [this] { return loopThreadIdReady; });
  }

  pool.initialize(poolSize);
  pool.$complete.subscribe([this](std::unique_ptr<BaseTask> task) {
    std::lock_guard<std::mutex> lock(resultMutex);
    result.push(std::move(task));
    tickCV.notify_one();
  });
}

void EventLoop::run() {
  std::unique_lock<std::mutex> lock(workDoneMutex);
  workDoneCV.wait(lock, [this] {
    return pending.load() == 0 || !running.load() || stopping.load();
  });
}

void EventLoop::tick() {
  {
    std::lock_guard<std::mutex> lock(loopThreadIdMutex);
    loopThreadId = std::this_thread::get_id();
    loopThreadIdReady = true;
    loopThreadIdCV.notify_all();
  }

  while (true) {
    if (tryToStartTask()) {
      continue;
    }

    if (tryToFinishTask()) {
      continue;
    }

    if (stopping.load() && pending.load() == 0) {
      return;
    }

    std::unique_lock<std::mutex> lock(tickMutex);
    tickCV.wait_for(lock, std::chrono::milliseconds(100));
  }
}

void EventLoop::push(std::unique_ptr<BaseTask> task) {
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

  auto task = std::move(const_cast<std::unique_ptr<BaseTask> &>(queue.top()));
  queue.pop();

  lock.unlock();

  if (task->isCancelRequested()) {
    task->markCancelled();
    std::lock_guard<std::mutex> resultLock(resultMutex);
    result.push(std::move(task));
    tickCV.notify_one();
    return true;
  }

  startTask(std::move(task));

  return true;
}

bool EventLoop::tryToFinishTask() {
  std::unique_lock<std::mutex> lock(resultMutex);
  if (result.empty()) {
    return false;
  }

  auto task = std::move(const_cast<std::unique_ptr<BaseTask> &>(result.top()));
  result.pop();

  lock.unlock();

  finishTask(std::move(task));

  return true;
}

void EventLoop::startTask(std::unique_ptr<BaseTask> task) {
  pool.schedule(std::move(task));
}

void EventLoop::finishTask(std::unique_ptr<BaseTask> task) {
  task->finish();

  try {
    task->cleanup();
  } catch (...) {
  }

  task->markDone();

  int old = pending.fetch_sub(1);

  if (old == 1) {
    workDoneCV.notify_one();
  }
}

void EventLoop::terminate() {
  if (stopping.exchange(true)) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!queue.empty()) {
      auto task =
          std::move(const_cast<std::unique_ptr<BaseTask> &>(queue.top()));
      queue.pop();
      task->requestCancel();
      task->markCancelled();
      std::lock_guard<std::mutex> resultLock(resultMutex);
      result.push(std::move(task));
    }
  }

  pool.requestStop();
  auto stolen = pool.stealQueued();
  if (!stolen.empty()) {
    std::lock_guard<std::mutex> resultLock(resultMutex);
    for (auto &task : stolen) {
      task->requestCancel();
      task->markCancelled();
      result.push(std::move(task));
    }
  }

  tickCV.notify_one();

  {
    std::unique_lock<std::mutex> lock(workDoneMutex);
    workDoneCV.wait(lock, [this] { return pending.load() == 0; });
  }

  running = false;
  tickCV.notify_one();
  workDoneCV.notify_all();

  if (eventLoopThread.joinable()) {
    eventLoopThread.join();
  }

  pool.join();
}

EventLoop::~EventLoop() { terminate(); }