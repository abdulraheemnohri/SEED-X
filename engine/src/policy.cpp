#include "seed/policy.hpp"
#include <unordered_set>
#include <utility>
namespace seed {
namespace { int precedence(PolicyEffect e){return e==PolicyEffect::Deny?3:(e==PolicyEffect::RequireApproval?2:1);} }
PolicyEngine::PolicyEngine(){rules_.push_back({"memory.read","read","memory://",PolicyEffect::Allow,true});rules_.push_back({"system.status","read","system://",PolicyEffect::Allow,true});rules_.push_back({"files.list","read","workspace://",PolicyEffect::Allow,true});rules_.push_back({"files.read","read","workspace://",PolicyEffect::Allow,true});rules_.push_back({"workspace.search","search","workspace://",PolicyEffect::Allow,true});rules_.push_back({"files.write","create","workspace://",PolicyEffect::RequireApproval,true});}
PolicyResult PolicyEngine::evaluate(const ToolRequest&r)const{
 if(r.tool.empty()||r.operation.empty()||r.resource.empty())return {Decision::Deny,"Tool, operation and resource are required."};
 const PolicyRule* selected=nullptr;
 for(const auto& rule:rules_){if(!rule.enabled||rule.tool!=r.tool||rule.operation!=r.operation)continue;if(r.resource.compare(0,rule.resourcePrefix.size(),rule.resourcePrefix)!=0)continue;
 if(!selected||rule.resourcePrefix.size()>selected->resourcePrefix.size()||(rule.resourcePrefix.size()==selected->resourcePrefix.size()&&precedence(rule.effect)>precedence(selected->effect)))selected=&rule;}
 if(!selected)return {Decision::Deny,"No matching policy rule. Default deny."};
 if(selected->effect==PolicyEffect::Allow)return {Decision::Allow,"Matched explicit allow policy."};
 if(selected->effect==PolicyEffect::RequireApproval)return {Decision::RequireApproval,"Matched policy requires explicit operator approval."};
 return {Decision::Deny,"Matched explicit deny policy."};
}
PolicyValidation PolicyEngine::validate(const std::vector<PolicyRule>& rules)const{
 if(rules.size()>256)return {false,"Policy exceeds the 256-rule limit."};
 std::unordered_set<std::string> keys;
 for(const auto&r:rules){if(r.tool.empty()||r.operation.empty()||r.resourcePrefix.empty())return {false,"Every rule requires tool, operation and resource prefix."};
 if(r.tool.size()>128||r.operation.size()>64||r.resourcePrefix.size()>256)return {false,"A policy field exceeds its maximum length."};
 if(r.tool.find_first_of(" \t\r\n")!=std::string::npos||r.operation.find_first_of(" \t\r\n")!=std::string::npos)return {false,"Tool and operation identifiers cannot contain whitespace."};
 const auto key=r.tool+"\n"+r.operation+"\n"+r.resourcePrefix;if(!keys.insert(key).second)return {false,"Duplicate policy rule detected."};}
 return {true,"Policy rules validated."};
}
PolicyValidation PolicyEngine::replaceRules(std::vector<PolicyRule> rules){const auto result=validate(rules);if(!result.valid)return result;rules_=std::move(rules);return {true,"Policy rules replaced."};}
const std::vector<PolicyRule>& PolicyEngine::rules()const noexcept{return rules_;}
}
