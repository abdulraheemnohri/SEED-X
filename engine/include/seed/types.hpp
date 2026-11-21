#pragma once
#include <chrono>
#include <cstdint>
#include <string>
namespace seed {
enum class AgentStatus : std::uint8_t { Stopped, Initializing, Ready, Running, Paused, Error };
enum class Decision : std::uint8_t { Allow, RequireApproval, Deny };
struct ToolRequest { std::string tool, operation, resource, reason; };
struct PolicyResult { Decision decision{Decision::Deny}; std::string explanation{"No policy decision was made."}; };
struct AgentEvent { std::string type, message; std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()}; };
}
