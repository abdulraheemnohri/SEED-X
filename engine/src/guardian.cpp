#include "seed/guardian.hpp"
#include <utility>
namespace seed {
Guardian::Guardian()=default;
std::string Guardian::fingerprint(const ToolRequest&r){return std::to_string(r.tool.size())+":"+r.tool+std::to_string(r.operation.size())+":"+r.operation+std::to_string(r.resource.size())+":"+r.resource;}
PolicyResult Guardian::evaluate(const ToolRequest&r)const{if(frozen_.load())return {Decision::Deny,"Emergency stop is active; all actions are blocked."};std::lock_guard<std::mutex> lock(mutex_);return policy_.evaluate(r);}
void Guardian::record(const ToolRequest&r,const PolicyResult&p){if(!audit_.append(r,p))frozen_.store(true);}
PolicyResult Guardian::authorize(const ToolRequest&r){PolicyResult result;if(frozen_.load())result={Decision::Deny,"Emergency stop is active; all actions are blocked."};else{std::lock_guard<std::mutex> lock(mutex_);result=policy_.evaluate(r);}record(r,result);if(frozen_.load()&&result.decision!=Decision::Deny)return {Decision::Deny,"Guardian failed closed because audit capacity was exhausted."};return result;}
PolicyResult Guardian::authorizeAndRun(const ToolRequest&r,const std::string&ticketId,const std::function<void()>&action){
 if(!action)return {Decision::Deny,"No tool handler was supplied."};
 std::lock_guard<std::mutex> lock(mutex_);
 if(frozen_.load())return {Decision::Deny,"Emergency stop is active; all actions are blocked."};
 const auto policy=policy_.evaluate(r);
 if(policy.decision==Decision::Deny){record(r,policy);return policy;}
 if(policy.decision==Decision::RequireApproval){
  if(ticketId.empty()){record(r,policy);return policy;}
  const auto it=approvals_.find(ticketId);
  if(it==approvals_.end()){const PolicyResult denied{Decision::Deny,"Approval ticket is unknown or already consumed."};record(r,denied);return denied;}
  const bool valid=it->second.expiresAt>std::chrono::system_clock::now()&&it->second.requestFingerprint==fingerprint(r);
  approvals_.erase(it);
  if(!valid){const PolicyResult denied{Decision::Deny,"Approval ticket expired or belongs to another request."};record(r,denied);return denied;}
  record(r,{Decision::Allow,"One-use operator approval consumed; action authorized."});
 } else record(r,policy);
 if(frozen_.load())return {Decision::Deny,"Guardian failed closed before tool execution."};
 action();
 return {Decision::Allow,"Guardian authorized and completed the registered action."};
}
PolicyValidation Guardian::replacePolicies(std::vector<PolicyRule> rules,bool confirmed){if(!confirmed)return {false,"Policy replacement requires operator confirmation."};if(frozen_.load())return {false,"Cannot change policies while emergency stop is active."};std::lock_guard<std::mutex> lock(mutex_);return policy_.replaceRules(std::move(rules));}
std::vector<PolicyRule> Guardian::policies()const{std::lock_guard<std::mutex> lock(mutex_);return policy_.rules();}
ApprovalTicket Guardian::requestApproval(const ToolRequest&r,bool confirmed,std::chrono::seconds lifetime){if(!confirmed||frozen_.load())return {};if(lifetime<std::chrono::seconds(1))lifetime=std::chrono::seconds(1);if(lifetime>std::chrono::seconds(300))lifetime=std::chrono::seconds(300);std::lock_guard<std::mutex> lock(mutex_);if(frozen_.load()||policy_.evaluate(r).decision!=Decision::RequireApproval)return {};const auto expiry=std::chrono::system_clock::now()+lifetime;const auto id="seed-approval-"+std::to_string(nextTicket_++);approvals_[id]={fingerprint(r),expiry};record(r,{Decision::RequireApproval,"Operator issued a one-use approval ticket."});return {id,expiry};}
bool Guardian::consumeApproval(const ToolRequest&r,const std::string&id){if(id.empty()||frozen_.load())return false;std::lock_guard<std::mutex> lock(mutex_);const auto it=approvals_.find(id);if(it==approvals_.end())return false;const bool valid=it->second.expiresAt>std::chrono::system_clock::now()&&it->second.requestFingerprint==fingerprint(r);approvals_.erase(it);record(r,{valid?Decision::Allow:Decision::Deny,valid?"One-use operator approval consumed.":"Approval ticket invalid, expired or bound to another request."});return valid&&!frozen_.load();}
void Guardian::emergencyStop()noexcept{frozen_.store(true);std::lock_guard<std::mutex> lock(mutex_);approvals_.clear();}
bool Guardian::resetEmergencyStop(bool confirmed)noexcept{if(!confirmed)return false;std::lock_guard<std::mutex> lock(mutex_);approvals_.clear();frozen_.store(false);return true;}
bool Guardian::isFrozen()const noexcept{return frozen_.load();}
bool Guardian::verifyAudit()const{return audit_.verify();}
std::vector<AuditEntry> Guardian::auditSnapshot()const{return audit_.snapshot();}
}
