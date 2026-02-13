#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "event_loop/runtime/runtime.h"
#include "test_utils.h"

TEST(AsyncRuntime, LaunchRunsAndHandleWaits) {
  AsyncRuntime runtime(1);

  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;
  std::thread::id runThreadId;

  TaskHandle handle = runtime.launch([&] {
    {
      std::lock_guard<std::mutex> lock(mutex);
      runThreadId = std::this_thread::get_id();
      done = true;
    }
    cv.notify_one();
  });

  ASSERT_TRUE(runtime_test::wait_for(cv, mutex, std::chrono::seconds(2),
                                    [&] { return done; }));

  handle.wait();
  runtime.blockOn();

  EXPECT_NE(runThreadId, runtime.loopThreadId());
  EXPECT_NE(runThreadId, std::this_thread::get_id());
}
