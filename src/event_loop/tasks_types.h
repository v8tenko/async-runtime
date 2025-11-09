#include <set>
#include <cstdint>
#include <memory>

#include "task/task.h"

using TaskQueue = std::set<std::pair<uint8_t, std::shared_ptr<Task>>>;
using TaskHandler = std::function<void(std::shared_ptr<Task>)>;