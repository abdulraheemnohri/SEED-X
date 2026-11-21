#pragma once
#include "seed/types.hpp"
#include <functional>
#include <mutex>
#include <vector>
namespace seed {
class EventBus final {
public:
 using Listener=std::function<void(const AgentEvent&)>;
 void subscribe(Listener listener);
 void publish(const AgentEvent& event) const;
private:
 mutable std::mutex mutex_;
 std::vector<Listener> listeners_;
};
}
