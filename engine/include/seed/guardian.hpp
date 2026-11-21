#pragma once
#include "seed/types.hpp"
#include <string>
#include <unordered_set>
namespace seed {
class Guardian final {
public:
 Guardian();
 PolicyResult evaluate(const ToolRequest& request) const;
 bool isFrozen() const noexcept;
 void freeze() noexcept;
 void unfreeze() noexcept;
private:
 bool frozen_{false};
 std::unordered_set<std::string> readOnlyTools_;
};
}
