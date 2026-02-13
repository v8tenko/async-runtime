#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "event_loop/runtime/runtime.h"
#include "test_utils.h"

TEST(AsyncRuntime, ReadFileReturnsContentAndCallbackOnLoopThread) {
  runtime_test::TempFile file("hello\nworld");

  AsyncRuntime runtime(1);

  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;

  std::optional<std::string> outError;
  std::optional<std::string> outValue;
  std::thread::id callbackThreadId;

  runtime.readFile(file.path.string(), [&](std::optional<std::string> error,
                                          std::optional<std::string> value) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      outError = std::move(error);
      outValue = std::move(value);
      callbackThreadId = std::this_thread::get_id();
      done = true;
    }
    cv.notify_one();
  });

  ASSERT_TRUE(runtime_test::wait_for(cv, mutex, std::chrono::seconds(2),
                                    [&] { return done; }));

  runtime.blockOn();

  EXPECT_FALSE(outError.has_value());
  ASSERT_TRUE(outValue.has_value());
  EXPECT_EQ(outValue.value(), std::string("hello\nworld\n"));
  EXPECT_EQ(callbackThreadId, runtime.loopThreadId());
}
