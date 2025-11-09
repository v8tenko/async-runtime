#include <iostream>

#include "event_loop/wrappers/async.h"


int main() {
	async::readFile(".gitignore", [](std::string result) {
		std::cout << "Hello from " << result << std::endl;
	});
	
	std::this_thread::sleep_for(std::chrono::seconds(100));

	return EXIT_SUCCESS;
}