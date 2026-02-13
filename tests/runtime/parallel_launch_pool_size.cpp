#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "event_loop/runtime/runtime.h"
#include "test_utils.h"

TEST(AsyncRuntime, LaunchRunsInParallelWhenPoolSizeAllows) {
  AsyncRuntime runtime(2);

  std::mutex mutex;
  std::condition_variable cv;
  int started = 0;
  bool release = false;

  auto blockingWork = [&] {
    {
      std::lock_guard<std::mutex> lock(mutex);
      started++;
      cv.notify_one();
    }

    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&] { return release; });
  };

  TaskHandle a = runtime.launch(blockingWork);
  TaskHandle b = runtime.launch(blockingWork);

  // Both tasks should start before we release them.
  ASSERT_TRUE(runtime_test::wait_for(cv, mutex, std::chrono::seconds(2),
                                    [&] { return started >= 2; }));

  {
    std::lock_guard<std::mutex> lock(mutex);
    release = true;
  }
  cv.notify_all();

  a.wait();
  b.wait();
  runtime.blockOn();

  SUCCEED();
}
