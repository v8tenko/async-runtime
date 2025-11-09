#include <functional>
#include <string>

#include "../event_loop.h"

namespace async {
	namespace detail {
		inline void prelude() {
			static bool initialized = false;

			if (initialized) {
				return;
			}

			initialized = true;
			EventLoop::instance().initialize(5);
		}
	}

	void readFile(std::string path, std::function<void(std::string)> cb);
	void run(std::function<void()> cb);
};