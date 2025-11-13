#include <string>

#include "../event_loop.h"
#include "../task/task.h"

class AsyncRuntime {
public:
	AsyncRuntime(uint8_t poolSize = 1) {
		loop.initialize(poolSize);
	};

	void blockOn() {
		loop.run();
	}

  void readFile(std::string path, Callback<std::string> cb);
  void run(std::function<void()> cb);

private:
	EventLoop loop;
};