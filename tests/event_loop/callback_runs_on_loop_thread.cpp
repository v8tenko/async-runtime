#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "event_loop/event_loop.h"

namespace {

class ThreadCheckTask : public Task<int> {
public:
  ThreadCheckTask(std::thread::id *executeId, std::thread::id *callbackId,
                  std::mutex *mutex, std::condition_variable *cv, bool *done,
                  uint8_t priority = 0)
      : Task<int>(
            Callback<int>([callbackId, mutex, cv, done](
                              std::optional<std::string> error,
                              std::optional<int> value) {
              (void)error;
              (void)value;
              *callbackId = std::this_thread::get_id();
              {
                std::lock_guard<std::mutex> lock(*mutex);
                *done = true;
              }
              cv->notify_one();
            }),
            priority),
        executeId(executeId) {}

  void execute() override {
    *executeId = std::this_thread::get_id();
    value = 123;
  }

  void cleanup() override {}

  std::string toString() const override { return "ThreadCheckTask"; }

private:
  std::thread::id *executeId;
};

} // namespace

TEST(EventLoop, CallbackRunsOnLoopThread) {
  EventLoop loop;
  loop.initialize(1);

  std::thread::id executeId;
  std::thread::id callbackId;

  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;

  loop.push(std::make_unique<ThreadCheckTask>(&executeId, &callbackId, &mutex,
                                              &cv, &done));

  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(2), [&] { return done; });
  }

  loop.run();

  EXPECT_NE(executeId, callbackId);
  EXPECT_EQ(callbackId, loop.threadId());
}
