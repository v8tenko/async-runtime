#include <functional>
#include <string>

#include "../event_loop.h"

class AsyncRuntime {
public:
	AsyncRuntime(uint8_t poolSize = 1) {
		loop.initialize(poolSize);
	};

	void blockOn() {
		loop.run();
	}

  void readFile(std::string path, std::function<void(std::string)> cb);
  void run(std::function<void()> cb);

private:
	EventLoop loop;
};