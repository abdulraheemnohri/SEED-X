#pragma once
#include "seed/types.hpp"
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
namespace seed {
struct AuditEntry {
 std::uint64_t sequence{0}; std::int64_t timestampMs{0};
 std::string tool,operation,resource; Decision decision{Decision::Deny};
 std::string reason,previousHash,hash;
};
class AuditLog final {
public:
 explicit AuditLog(std::size_t capacity=4096);
 bool append(const ToolRequest&,const PolicyResult&);
 bool verify() const;
 std::vector<AuditEntry> snapshot() const;
 std::size_t size() const;
private:
 static std::string calculateHash(const AuditEntry&);
 mutable std::mutex mutex_; std::vector<AuditEntry> entries_; std::size_t capacity_;
};
}
