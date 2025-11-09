#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "../tasks_types.h"

struct ThreadsPool {
	void initialize(uint8_t count);

	void queue(std::shared_ptr<Task> task);
	void onComplete(TaskHandler handler);

	~ThreadsPool();

	private:
		std::vector<std::thread> threads;
		std::vector<TaskHandler> handlers;

		std::mutex mutex;
		std::deque<std::shared_ptr<Task>> _queue;

		std::atomic<bool> running = true;

		void run();
};