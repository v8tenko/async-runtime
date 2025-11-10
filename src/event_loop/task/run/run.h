#include <functional>

#include "../task.h"

class RunTask : public Task {
public:
  RunTask(std::function<void()> cb, uint8_t priority = 1);

  void execute() override;
	void cleanup() override;

  std::string toString() const override;

private:
  std::function<void()> cb;

protected:
  void complete() override;
};