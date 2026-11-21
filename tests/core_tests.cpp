#include "seed/agent_kernel.hpp"
#include <cassert>
#include <iostream>
int main(){seed::AgentKernel k;assert(k.status()==seed::AgentStatus::Stopped);assert(k.initialize());assert(k.status()==seed::AgentStatus::Ready);assert(k.start());assert(k.status()==seed::AgentStatus::Running);auto ok=k.authorize({"memory.read","read","memory://recent","read"});assert(ok.decision==seed::Decision::Allow);auto no=k.authorize({"shell","execute","system","run command"});assert(no.decision==seed::Decision::Deny);k.pause();assert(k.status()==seed::AgentStatus::Paused);assert(k.start());k.stop();assert(k.status()==seed::AgentStatus::Stopped);std::cout<<"SEED-X core tests passed\\n";}
