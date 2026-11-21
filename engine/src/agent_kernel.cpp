#include "seed/agent_kernel.hpp"
#include <utility>
namespace seed {
bool AgentKernel::initialize(){AgentStatus expected=AgentStatus::Stopped;if(!status_.compare_exchange_strong(expected,AgentStatus::Initializing))return expected==AgentStatus::Ready;status_.store(AgentStatus::Ready);events_.publish({"kernel.ready","SEED-X native kernel initialized."});return true;}
bool AgentKernel::start(){if(guardian_.isFrozen())return false;auto current=status_.load();if(current==AgentStatus::Stopped&&!initialize())return false;current=status_.load();if(guardian_.isFrozen()||(current!=AgentStatus::Ready&&current!=AgentStatus::Paused))return false;status_.store(AgentStatus::Running);events_.publish({"kernel.started","Agent kernel started."});return true;}
void AgentKernel::pause(){AgentStatus expected=AgentStatus::Running;if(status_.compare_exchange_strong(expected,AgentStatus::Paused))events_.publish({"kernel.paused","Agent kernel paused."});}
void AgentKernel::stop(){status_.store(AgentStatus::Stopped);events_.publish({"kernel.stopped","Agent kernel stopped."});}
AgentStatus AgentKernel::status()const noexcept{return status_.load();}\nLocalAiRuntime& AgentKernel::localAi()noexcept{return localAi_;}
PolicyResult AgentKernel::authorize(const ToolRequest&r){return guardian_.authorize(r);}
PolicyValidation AgentKernel::replacePolicies(std::vector<PolicyRule> r,bool confirmed){return guardian_.replacePolicies(std::move(r),confirmed);}
ApprovalTicket AgentKernel::requestApproval(const ToolRequest&r,bool c){return guardian_.requestApproval(r,c);}
bool AgentKernel::consumeApproval(const ToolRequest&r,const std::string&id){return guardian_.consumeApproval(r,id);}
void AgentKernel::emergencyStop(){guardian_.emergencyStop();status_.store(AgentStatus::Paused);events_.publish({"guardian.emergency_stop","Emergency stop engaged; tool authorization is blocked."});}
bool AgentKernel::resetEmergencyStop(bool c){const bool ok=guardian_.resetEmergencyStop(c);if(ok)events_.publish({"guardian.emergency_stop_reset","Emergency stop reset by operator confirmation."});return ok;}
bool AgentKernel::verifyAudit()const{return guardian_.verifyAudit();}
EventBus& AgentKernel::events()noexcept{return events_;}
}
