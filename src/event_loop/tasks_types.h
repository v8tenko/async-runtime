#pragma once

#include <memory>
#include <queue>

#include "task/task.h"

class TaskComparator {
public:
  bool operator()(const std::shared_ptr<BaseTask> &a,
                  const std::shared_ptr<BaseTask> &b) const noexcept {
    return a->priority < b->priority;
  }
};

using TaskQueue =
    std::priority_queue<std::shared_ptr<BaseTask>,
                        std::vector<std::shared_ptr<BaseTask>>, TaskComparator>;
