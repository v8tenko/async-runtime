#include <functional>
#include <string>

#include "../event_loop.h"

namespace async {
	namespace detail {
		inline void prelude() {
			EventLoop::instance().start();
		}
	}

	void readFile(const std::string& path, std::function<void(std::string)> cb);
};