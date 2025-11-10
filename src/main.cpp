#include <iostream>

#include "event_loop/runtime/runtime.h"

int main() {
  AsyncRuntime runtime;

  runtime.readFile(".gitignore", [](std::string result) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello from " << result.length() << std::endl;
  });

  runtime.run([] {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Hello from thread " << std::this_thread::get_id()
              << std::endl;
  });

  std::cout << "First!" << std::endl;

  runtime.blockOn();

  std::cout << "Tasks completed" << std::endl;

  runtime.run([] {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Hello from thread 2 " << std::this_thread::get_id()
              << std::endl;
  });

  std::cout << "Task spawned" << std::endl;

  runtime.blockOn();

  return EXIT_SUCCESS;
}