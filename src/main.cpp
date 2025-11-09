#include <iostream>

#include "event_loop/wrappers/async.h"


int main() {
	async::readFile(".gitignore", [](std::string result) {
		std::cout << "Hello from " << result << std::endl;
	});
	
	EventLoop::instance().run();

	return EXIT_SUCCESS;
}