#pragma once
#include "seed/agent_kernel.hpp"
#include <functional>
#include <string>
#include <vector>
namespace seed {
struct ToolResult { bool success{false}; Decision decision{Decision::Deny}; std::string output; };
struct ToolDescriptor { std::string tool, operation, resourcePrefix, description; };
class ToolRegistry final {
public:
 using Handler = std::function<ToolResult(const ToolRequest&, const std::string&)>;
 bool add(ToolDescriptor descriptor, Handler handler);
 ToolResult execute(AgentKernel& kernel, const ToolRequest& request, const std::string& payload={}, const std::string& approvalTicket={}) const;
 std::vector<ToolDescriptor> describe() const;
private:
 struct Entry { ToolDescriptor descriptor; Handler handler; };
 std::vector<Entry> entries_;
};
}
