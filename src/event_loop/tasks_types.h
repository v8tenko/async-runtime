#pragma once

#include <memory>
#include <queue>

#include "task/task.h"

class TaskComparator {
public:
  bool operator()(const std::shared_ptr<Task> &a,
                  const std::shared_ptr<Task> &b) const noexcept {
    return a->priority < b->priority;
  }
};

using TaskQueue =
    std::priority_queue<std::shared_ptr<Task>,
                        std::vector<std::shared_ptr<Task>>, TaskComparator>;

using TaskHandler = std::function<void(std::shared_ptr<Task>)>;