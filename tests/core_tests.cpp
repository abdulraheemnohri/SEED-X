#include "seed/agent_kernel.hpp"
#include "seed/tool_registry.hpp"
#include <cassert>
#include <iostream>
int main(){
 seed::AgentKernel k;assert(k.status()==seed::AgentStatus::Stopped);assert(k.initialize());assert(k.start());
 const seed::ToolRequest read{"memory.read","read","memory://recent","read memory"};
 const seed::ToolRequest shell{"shell","execute","system://root","run command"};
 assert(k.authorize(read).decision==seed::Decision::Allow);assert(k.authorize(shell).decision==seed::Decision::Deny);
 auto proposed=std::vector<seed::PolicyRule>{{"files.write","write","files://documents/",seed::PolicyEffect::RequireApproval,true},{"files.write","write","files://documents/private/",seed::PolicyEffect::Deny,true}};
 assert(!k.replacePolicies(proposed,false).valid);
 auto set=k.replacePolicies(proposed,true);
 assert(set.valid);
 const seed::ToolRequest write{"files.write","write","files://documents/report.txt","save report"};
 const seed::ToolRequest secret{"files.write","write","files://documents/private/secret.txt","save secret"};
 assert(k.authorize(write).decision==seed::Decision::RequireApproval);assert(k.authorize(secret).decision==seed::Decision::Deny);
 assert(k.requestApproval(write,false).id.empty());
 auto t=k.requestApproval(write,true);assert(!t.id.empty());assert(!k.consumeApproval(secret,t.id));assert(!k.consumeApproval(write,t.id));
 auto t2=k.requestApproval(write,true);assert(!t2.id.empty());assert(k.consumeApproval(write,t2.id));assert(!k.consumeApproval(write,t2.id));
 assert(k.authorize(read).decision==seed::Decision::Deny);
 assert(!k.replacePolicies({{"","read","memory://",seed::PolicyEffect::Allow,true}},true).valid);
 assert(k.verifyAudit());
 k.emergencyStop();assert(k.emergencyStopped());assert(k.authorize(write).decision==seed::Decision::Deny);assert(!k.start());assert(!k.resetEmergencyStop(false));assert(k.resetEmergencyStop(true));assert(!k.emergencyStopped());
 assert(k.verifyAudit());k.pause();assert(k.status()==seed::AgentStatus::Paused);assert(k.start());k.stop();assert(k.status()==seed::AgentStatus::Stopped);
 auto toolPolicy=k.replacePolicies({{"files.write","create","workspace://",seed::PolicyEffect::RequireApproval,true}},true);
 assert(toolPolicy.valid);
 seed::ToolRegistry tools;
 int writes=0;
 assert(tools.add({"files.write","create","workspace://","Create a workspace note"},
  [&](const seed::ToolRequest&,const std::string& payload){++writes;return seed::ToolResult{true,seed::Decision::Allow,payload};}));
 assert(!tools.add({"files.write","create","workspace://","duplicate"},[](const seed::ToolRequest&,const std::string&){return seed::ToolResult{};}));
 const seed::ToolRequest note{"files.write","create","workspace://notes.txt","create note"};
 auto denied=tools.execute(k,note,"hello");
 assert(!denied.success&&denied.decision==seed::Decision::RequireApproval&&writes==0);
 auto approval=k.requestApproval(note,true);
 assert(!approval.id.empty());
 auto executed=tools.execute(k,note,"hello",approval.id);
 assert(executed.success&&executed.output=="hello"&&writes==1);
 auto replay=tools.execute(k,note,"hello",approval.id);
 assert(!replay.success&&writes==1);
 const seed::ToolRequest unknown{"shell","execute","system://root","never run shell"};
 assert(!tools.execute(k,unknown).success);
 bool sawCompleted=false,sawFailure=false;for(const auto& entry:k.auditSnapshot()){if(entry.reason=="Registered tool handler completed successfully.")sawCompleted=true;if(entry.reason=="Registered tool handler failed.")sawFailure=true;}
 assert(sawCompleted&&sawFailure);
 assert(k.verifyAudit());
 k.emergencyStop();auto stopped=tools.execute(k,note,"must not run");assert(!stopped.success&&writes==1);assert(k.resetEmergencyStop(true));
 assert(k.verifyAudit());
 std::cout<<"SEED-X core and Phase 7 tool registry tests passed\n";
}
