#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>

#include "event_loop/event_loop.h"

namespace {

class CleanupOnErrorTask : public Task<int> {
public:
  CleanupOnErrorTask(std::atomic<bool> *cleaned, std::optional<std::string> *out,
                     std::mutex *mutex, std::condition_variable *cv, bool *done,
                     uint8_t priority = 0)
      : Task<int>(
            Callback<int>([out, mutex, cv, done](std::optional<std::string> err,
                                                std::optional<int> value) {
              (void)value;
              *out = err;
              {
                std::lock_guard<std::mutex> lock(*mutex);
                *done = true;
              }
              cv->notify_one();
            }),
            priority),
        cleaned(cleaned) {}

  void execute() override { throw std::runtime_error("boom"); }

  void cleanup() override { cleaned->store(true); }

  std::string toString() const override { return "CleanupOnErrorTask"; }

private:
  std::atomic<bool> *cleaned;
};

} // namespace

TEST(EventLoop, CleanupRunsOnError) {
  EventLoop loop;
  loop.initialize(1);

  std::atomic<bool> cleaned = false;
  std::optional<std::string> error;
  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;

  loop.push(std::make_unique<CleanupOnErrorTask>(&cleaned, &error, &mutex, &cv,
                                                 &done));

  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(2), [&] { return done; });
  }

  loop.run();

  EXPECT_TRUE(cleaned.load());
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error.value(), "boom");
}
