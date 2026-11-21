#include "seed/event_bus.hpp"
namespace seed {
void EventBus::subscribe(Listener listener){if(!listener)return;std::lock_guard lock(mutex_);listeners_.push_back(std::move(listener));}
void EventBus::publish(const AgentEvent& event) const {
 std::vector<Listener> snapshot; {std::lock_guard lock(mutex_);snapshot=listeners_;}
 for(const auto& listener:snapshot){try{listener(event);}catch(...){/* Subscribers cannot crash the kernel. */}}
}
}
