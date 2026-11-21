#pragma once
#include "seed/types.hpp"
#include <string>
#include <vector>
namespace seed {
enum class PolicyEffect : std::uint8_t { Allow, RequireApproval, Deny };
struct PolicyRule { std::string tool, operation, resourcePrefix; PolicyEffect effect{PolicyEffect::Deny}; bool enabled{true}; };
struct PolicyValidation { bool valid{false}; std::string message; };
class PolicyEngine final {
public:
 PolicyEngine();
 PolicyResult evaluate(const ToolRequest&) const;
 PolicyValidation validate(const std::vector<PolicyRule>&) const;
 PolicyValidation replaceRules(std::vector<PolicyRule>);
 const std::vector<PolicyRule>& rules() const noexcept;
private: std::vector<PolicyRule> rules_;
};
}
