#include <string>
#include <thread>

#include "../event_loop.h"
#include "../task/task.h"

class AsyncRuntime {
public:
  AsyncRuntime(uint8_t poolSize = 1) { loop.initialize(poolSize); };

  void blockOn() { loop.run(); }

  TaskHandle readFile(std::string path, Callback<std::string> cb);
  TaskHandle launch(std::function<void()> cb);

  std::thread::id loopThreadId() const { return loop.threadId(); }

private:
  EventLoop loop;
};