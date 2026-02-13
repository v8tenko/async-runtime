#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <string>

#include "event_loop/event_loop.h"

namespace {

class GateTask : public Task<std::monostate> {
public:
  GateTask(std::shared_future<void> gate, uint8_t priority = 1)
      : Task<std::monostate>(std::nullopt, priority), gate(std::move(gate)) {}

  void execute() override { gate.wait(); }

  void cleanup() override {}

  std::string toString() const override { return "GateTask"; }

private:
  std::shared_future<void> gate;
};

class CancelledTask : public Task<int> {
public:
  CancelledTask(std::atomic<bool> *executed, std::optional<std::string> *outErr,
                std::mutex *mutex, std::condition_variable *cv, bool *done,
                uint8_t priority = 0)
      : Task<int>(
            Callback<int>([outErr, mutex, cv, done](
                              std::optional<std::string> error,
                              std::optional<int> value) {
              (void)value;
              *outErr = error;
              {
                std::lock_guard<std::mutex> lock(*mutex);
                *done = true;
              }
              cv->notify_one();
            }),
            priority),
        executed(executed) {}

  void execute() override {
    executed->store(true);
    value = 1;
  }

  void cleanup() override {}

  std::string toString() const override { return "CancelledTask"; }

private:
  std::atomic<bool> *executed;
};

} // namespace

TEST(EventLoop, CancelQueuedTask) {
  EventLoop loop;
  loop.initialize(1);

  std::promise<void> gatePromise;
  auto gate = gatePromise.get_future().share();

  loop.push(std::make_unique<GateTask>(gate));

  std::atomic<bool> executed = false;
  std::optional<std::string> error;
  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;

  auto cancelTask = std::make_unique<CancelledTask>(&executed, &error, &mutex,
                                                    &cv, &done);
  TaskHandle handle = cancelTask->handle();

  loop.push(std::move(cancelTask));
  handle.cancel();

  gatePromise.set_value();

  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(2), [&] { return done; });
  }

  loop.run();

  EXPECT_FALSE(executed.load());
  ASSERT_TRUE(error.has_value());
  EXPECT_EQ(error.value(), "Cancelled");
}
