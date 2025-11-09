#include <iostream>

#include "event_loop/wrappers/async.h"


int main() {
	async::readFile(".gitignore", [](std::string result) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Hello from " << result << std::endl;
	});

	async::run([] {
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(330));
			std::cout << "Hello from thread " << std::this_thread::get_id() << std::endl;
		}
	});

	std::cout << "First!" << std::endl;
	
	EventLoop::instance().run();

	return EXIT_SUCCESS;
}