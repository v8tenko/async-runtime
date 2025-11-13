#include <functional>
#include <shared_mutex>

template <typename Value>
using Listener = std::function<void(Value)>;

template <typename Value>
class Observable {
	public:
		void subscribe(Listener<Value> handler) {
    	std::shared_lock<std::shared_mutex> lock(mutex);
			
			listeners.push_back(std::move(handler));
		};

		void trigger(const Value& value) const {
    	std::shared_lock<std::shared_mutex> lock(mutex);
			
			for (const auto& listener: listeners) {
				listener(value);
			}
		}
	private:
		std::vector<Listener<Value>> listeners;
		mutable std::shared_mutex mutex;
};