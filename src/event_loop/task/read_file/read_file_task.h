#include <fstream>
#include <string>

#include "../task.h"

class ReadFileTask : public Task {
public:
  ReadFileTask(std::string path, std::function<void(std::string)> cb,
               uint8_t priority = 0);

  void execute() override;
  void cleanup() override;

  std::string toString() const override;

protected:
  void complete() override;

private:
  std::function<void(std::string)> cb;
  std::string path;
  std::ifstream file;
  std::unique_ptr<std::string> result;
};
