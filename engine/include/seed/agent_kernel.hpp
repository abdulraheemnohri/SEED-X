#pragma once
#include "seed/event_bus.hpp"
#include "seed/guardian.hpp"
#include "seed/types.hpp"
#include <atomic>
namespace seed {
class AgentKernel final {
public:
 bool initialize(); bool start(); void pause(); void stop();
 AgentStatus status() const noexcept;
 PolicyResult authorize(const ToolRequest& request) const;
 EventBus& events() noexcept;
private:
 std::atomic<AgentStatus> status_{AgentStatus::Stopped};
 Guardian guardian_; EventBus events_;
};
}
