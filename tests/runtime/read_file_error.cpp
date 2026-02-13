#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>

#include "event_loop/runtime/runtime.h"
#include "test_utils.h"

TEST(AsyncRuntime, ReadFileErrorPropagates) {
  AsyncRuntime runtime(1);

  const std::string missing =
      (std::filesystem::temp_directory_path() / "async_runtime_missing.txt")
          .string();

  std::mutex mutex;
  std::condition_variable cv;
  bool done = false;

  std::optional<std::string> outError;
  std::optional<std::string> outValue;

  runtime.readFile(missing, [&](std::optional<std::string> error,
                               std::optional<std::string> value) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      outError = std::move(error);
      outValue = std::move(value);
      done = true;
    }
    cv.notify_one();
  });

  ASSERT_TRUE(runtime_test::wait_for(cv, mutex, std::chrono::seconds(2),
                                    [&] { return done; }));

  runtime.blockOn();

  ASSERT_TRUE(outError.has_value());
  EXPECT_EQ(outError.value(), "Cannot open file: " + missing);
  EXPECT_FALSE(outValue.has_value());
}
