#pragma once
#include "seed/event_bus.hpp"
#include "seed/guardian.hpp"
#include "seed/local_ai.hpp"
#include <atomic>
#include <vector>
namespace seed {
class AgentKernel final {
public:
 bool initialize();bool start();void pause();void stop();AgentStatus status() const noexcept;
 LocalAiRuntime& localAi() noexcept;
 PolicyResult authorize(const ToolRequest&);
 PolicyValidation replacePolicies(std::vector<PolicyRule>,bool operatorConfirmed);
 ApprovalTicket requestApproval(const ToolRequest&,bool operatorConfirmed);
 bool consumeApproval(const ToolRequest&,const std::string&);
 void emergencyStop();bool resetEmergencyStop(bool operatorConfirmed);bool verifyAudit() const;bool emergencyStopped() const noexcept;
 EventBus& events() noexcept;
private: std::atomic<AgentStatus> status_{AgentStatus::Stopped};Guardian guardian_;EventBus events_;LocalAiRuntime localAi_;
};
}
