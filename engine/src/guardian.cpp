#include "seed/guardian.hpp"
namespace seed {
Guardian::Guardian(){ readOnlyTools_.insert("memory.read"); readOnlyTools_.insert("system.status"); }
PolicyResult Guardian::evaluate(const ToolRequest& r) const {
 if(frozen_) return {Decision::Deny,"Guardian is frozen; all tool execution is blocked."};
 if(r.tool.empty()||r.operation.empty()) return {Decision::Deny,"Tool name and operation are required."};
 if(readOnlyTools_.contains(r.tool)&&r.operation=="read") return {Decision::Allow,"Approved read-only Phase 1 capability."};
 return {Decision::Deny,"No explicit Phase 1 policy grants this operation. Default deny."};
}
bool Guardian::isFrozen() const noexcept{return frozen_;}
void Guardian::freeze() noexcept{frozen_=true;}
void Guardian::unfreeze() noexcept{frozen_=false;}
}
