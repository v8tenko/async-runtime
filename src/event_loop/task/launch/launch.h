#include <functional>
#include <variant>

#include "../task.h"

class RunTask : public Task<std::monostate> {
public:
  RunTask(std::function<void()> run, uint8_t priority = 1);

  void execute() override;
  void cleanup() override;

  std::string toString() const override;

private:
  std::function<void()> run;
};