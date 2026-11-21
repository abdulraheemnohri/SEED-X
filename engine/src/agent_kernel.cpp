#include "seed/agent_kernel.hpp"
namespace seed {
bool AgentKernel::initialize(){AgentStatus expected=AgentStatus::Stopped;if(!status_.compare_exchange_strong(expected,AgentStatus::Initializing))return expected==AgentStatus::Ready;status_.store(AgentStatus::Ready);events_.publish({"kernel.ready","SEED-X native kernel initialized."});return true;}
bool AgentKernel::start(){auto current=status_.load();if(current==AgentStatus::Stopped&&!initialize())return false;current=status_.load();if(current!=AgentStatus::Ready&&current!=AgentStatus::Paused)return false;status_.store(AgentStatus::Running);events_.publish({"kernel.started","Agent kernel started."});return true;}
void AgentKernel::pause(){AgentStatus expected=AgentStatus::Running;if(status_.compare_exchange_strong(expected,AgentStatus::Paused))events_.publish({"kernel.paused","Agent kernel paused."});}
void AgentKernel::stop(){status_.store(AgentStatus::Stopped);events_.publish({"kernel.stopped","Agent kernel stopped."});}
AgentStatus AgentKernel::status()const noexcept{return status_.load();}
PolicyResult AgentKernel::authorize(const ToolRequest&r)const{return guardian_.evaluate(r);}
EventBus& AgentKernel::events()noexcept{return events_;}
}
