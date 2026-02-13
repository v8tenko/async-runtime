#include <functional>
#include <shared_mutex>
#include <utility>

template <typename Value> using Listener = std::function<void(Value)>;

template <typename Value> class Observable {
public:
  void subscribe(Listener<Value> handler) {
    std::lock_guard<std::shared_mutex> lock(mutex);

    listener = std::make_optional(handler);
  };

  void trigger(Value value) const {
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (!listener.has_value()) {
      return;
    }

    listener.value()(std::move(value));
  }

private:
  std::optional<std::function<void(Value)>> listener;
  mutable std::shared_mutex mutex;
};