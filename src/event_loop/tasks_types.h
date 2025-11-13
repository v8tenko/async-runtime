#pragma once

#include <memory>
#include <queue>

#include "task/task.h"

class TaskComparator {
public:
  bool operator()(const std::unique_ptr<BaseTask> &a,
                  const std::unique_ptr<BaseTask> &b) const noexcept {
    return a->priority < b->priority;
  }
};

using TaskQueue =
    std::priority_queue<std::unique_ptr<BaseTask>,
                        std::vector<std::unique_ptr<BaseTask>>, TaskComparator>;
