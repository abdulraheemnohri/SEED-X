#include "seed/tool_registry.hpp"
#include <utility>
#include <exception>
namespace seed {
bool ToolRegistry::add(ToolDescriptor descriptor, Handler handler) {
 if(descriptor.tool.empty()||descriptor.operation.empty()||descriptor.resourcePrefix.empty()||!handler) return false;
 for(const auto& entry:entries_) {
  if(entry.descriptor.tool==descriptor.tool && entry.descriptor.operation==descriptor.operation &&
     entry.descriptor.resourcePrefix==descriptor.resourcePrefix) return false;
 }
 entries_.push_back({std::move(descriptor),std::move(handler)});
 return true;
}
ToolResult ToolRegistry::execute(AgentKernel& kernel,const ToolRequest& request,const std::string& payload,const std::string& approvalTicket) const {
 const Entry* selected=nullptr;
 for(const auto& entry:entries_) {
  const auto& d=entry.descriptor;
  if(d.tool==request.tool&&d.operation==request.operation&&
     request.resource.compare(0,d.resourcePrefix.size(),d.resourcePrefix)==0) {
   if(!selected||d.resourcePrefix.size()>selected->descriptor.resourcePrefix.size()) selected=&entry;
  }
 }
 if(!selected) {
  kernel.authorize(request);
  return {false,Decision::Deny,"Tool is not registered. No handler was executed."};
 }
 try {
  ToolResult result;
  const auto authorization=kernel.authorizeAndRun(request,approvalTicket,[&]{
   result=selected->handler(request,payload);
  });
  if(authorization.decision!=Decision::Allow)
   return {false,authorization.decision,authorization.explanation};
  result.decision=result.success?Decision::Allow:Decision::Deny;
  return result;
 } catch(const std::exception& e) {
  return {false,Decision::Deny,std::string("Tool handler failed safely: ")+e.what()};
 } catch(...) {
  return {false,Decision::Deny,"Tool handler failed with an unknown error."};
 }
}
std::vector<ToolDescriptor> ToolRegistry::describe() const {
 std::vector<ToolDescriptor> result;
 result.reserve(entries_.size());
 for(const auto& entry:entries_) result.push_back(entry.descriptor);
 return result;
}
}
