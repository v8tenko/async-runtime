#include <fstream>
#include <string>

#include "../task.h"

class ReadFileTask : public Task<std::string> {
public:
  ReadFileTask(std::string path, Callback<std::string> cb,
               uint8_t priority = 0);

  void execute() override;
  void cleanup() override;

  std::string toString() const override;

private:
  std::function<void(std::string)> cb;
  std::string path;
  std::ifstream file;
};
