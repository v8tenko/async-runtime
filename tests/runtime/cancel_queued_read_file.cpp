#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "event_loop/runtime/runtime.h"
#include "test_utils.h"

TEST(AsyncRuntime, CancelQueuedReadFileDoesNotExecute) {
  runtime_test::TempFile file("data");
  AsyncRuntime runtime(1);

  std::promise<void> gatePromise;
  auto gate = gatePromise.get_future().share();

  // Occupy the single worker so the readFile stays queued.
  TaskHandle gateHandle = runtime.launch([gate] { gate.wait(); });

  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;

  std::optional<std::string> outError;
  std::optional<std::string> outValue;
  std::thread::id callbackThreadId;

  TaskHandle handle = runtime.readFile(
      file.path.string(),
      [&](std::optional<std::string> error, std::optional<std::string> value) {
        {
          std::lock_guard<std::mutex> lock(mutex);
          outError = std::move(error);
          outValue = std::move(value);
          callbackThreadId = std::this_thread::get_id();
          done = true;
        }
        cv.notify_one();
      });

  handle.cancel();
  gatePromise.set_value();

  ASSERT_TRUE(runtime_test::wait_for(cv, mutex, std::chrono::seconds(2),
                                    [&] { return done; }));

  gateHandle.wait();
  runtime.blockOn();

  ASSERT_TRUE(outError.has_value());
  EXPECT_EQ(outError.value(), "Cancelled");
  EXPECT_FALSE(outValue.has_value());
  EXPECT_EQ(callbackThreadId, runtime.loopThreadId());
}
