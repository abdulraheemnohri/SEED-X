#include "seed/agent_kernel.hpp"
#include <cassert>
#include <iostream>
int main(){
 seed::AgentKernel k;assert(k.status()==seed::AgentStatus::Stopped);assert(k.initialize());assert(k.start());
 const seed::ToolRequest read{"memory.read","read","memory://recent","read memory"};
 const seed::ToolRequest shell{"shell","execute","system://root","run command"};
 assert(k.authorize(read).decision==seed::Decision::Allow);assert(k.authorize(shell).decision==seed::Decision::Deny);
 auto set=k.replacePolicies({{"files.write","write","files://documents/",seed::PolicyEffect::RequireApproval,true},{"files.write","write","files://documents/private/",seed::PolicyEffect::Deny,true}});
 assert(set.valid);
 const seed::ToolRequest write{"files.write","write","files://documents/report.txt","save report"};
 const seed::ToolRequest secret{"files.write","write","files://documents/private/secret.txt","save secret"};
 assert(k.authorize(write).decision==seed::Decision::RequireApproval);assert(k.authorize(secret).decision==seed::Decision::Deny);
 assert(k.requestApproval(write,false).id.empty());
 auto t=k.requestApproval(write,true);assert(!t.id.empty());assert(!k.consumeApproval(secret,t.id));assert(!k.consumeApproval(write,t.id));
 auto t2=k.requestApproval(write,true);assert(!t2.id.empty());assert(k.consumeApproval(write,t2.id));assert(!k.consumeApproval(write,t2.id));
 assert(k.authorize(read).decision==seed::Decision::Deny);
 assert(!k.replacePolicies({{"","read","memory://",seed::PolicyEffect::Allow,true}}).valid);
 assert(k.verifyAudit());
 k.emergencyStop();assert(k.authorize(write).decision==seed::Decision::Deny);assert(!k.resetEmergencyStop(false));assert(k.resetEmergencyStop(true));
 assert(k.verifyAudit());k.pause();assert(k.status()==seed::AgentStatus::Paused);assert(k.start());k.stop();assert(k.status()==seed::AgentStatus::Stopped);
 std::cout<<"SEED-X Phase 2 tests passed\n";
}
