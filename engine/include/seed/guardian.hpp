#pragma once
#include "seed/audit_log.hpp"
#include "seed/policy.hpp"
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
namespace seed {
struct ApprovalTicket { std::string id; std::chrono::system_clock::time_point expiresAt{}; };
class Guardian final {
public:
 Guardian();
 PolicyResult evaluate(const ToolRequest&) const;
 PolicyResult authorize(const ToolRequest&);
 PolicyValidation replacePolicies(std::vector<PolicyRule>);
 std::vector<PolicyRule> policies() const;
 ApprovalTicket requestApproval(const ToolRequest&,bool operatorConfirmed,std::chrono::seconds lifetime=std::chrono::seconds(60));
 bool consumeApproval(const ToolRequest&,const std::string& ticketId);
 void emergencyStop() noexcept;
 bool resetEmergencyStop(bool operatorConfirmed) noexcept;
 bool isFrozen() const noexcept;
 bool verifyAudit() const;
 std::vector<AuditEntry> auditSnapshot() const;
private:
 static std::string fingerprint(const ToolRequest&);
 void record(const ToolRequest&,const PolicyResult&);
 std::atomic<bool> frozen_{false}; mutable std::mutex mutex_; PolicyEngine policy_; AuditLog audit_;
 std::uint64_t nextTicket_{1};
 struct PendingApproval {std::string requestFingerprint;std::chrono::system_clock::time_point expiresAt;};
 std::unordered_map<std::string,PendingApproval> approvals_;
};
}
