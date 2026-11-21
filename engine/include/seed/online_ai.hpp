#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace seed {
enum class ProviderKind : std::uint8_t { OpenAICompatible, Anthropic, Gemini };
enum class ChatRole : std::uint8_t { System, User, Assistant };
struct ChatMessage { ChatRole role{ChatRole::User}; std::string content; };
struct ProviderConfig { std::string id,displayName,baseUrl,model,credentialRef; ProviderKind kind{ProviderKind::OpenAICompatible}; bool enabled{true}; std::uint32_t timeoutMs{30000}; std::size_t maxResponseBytes{2U*1024U*1024U}; bool allowInsecureLocalhost{false}; };
struct ChatRequest { std::vector<ChatMessage> messages; std::uint32_t maxOutputTokens{512}; float temperature{0.7F}; };
struct ProviderUsage { std::uint64_t inputTokens{0},outputTokens{0}; bool reportedByProvider{false}; };
struct ProviderResult { bool success{false}; std::string text,error; ProviderUsage usage; };
struct HttpRequest { std::string url,body; std::map<std::string,std::string> headers; std::uint32_t timeoutMs{30000}; std::size_t maxResponseBytes{2U*1024U*1024U}; };
struct HttpResponse { std::uint32_t statusCode{0}; std::string body; };
class HttpTransport { public: virtual ~HttpTransport()=default; virtual bool postJson(const HttpRequest&,HttpResponse&,std::string& error)=0; };
class CredentialVault { public: virtual ~CredentialVault()=default; virtual bool getSecret(const std::string& reference,std::string& secret) const=0; };
class ProviderAdapter { public: virtual ~ProviderAdapter()=default; virtual ProviderKind kind() const noexcept=0; virtual ProviderResult generate(const ProviderConfig&,const ChatRequest&,const HttpTransport&,const CredentialVault&) const=0; };
class ProviderRegistry final { public: bool add(ProviderConfig,std::string&); bool remove(const std::string&); const ProviderConfig* find(const std::string&) const noexcept; std::vector<ProviderConfig> enabled() const; std::size_t size() const noexcept; private: std::vector<ProviderConfig> providers_; };
enum class RouteTarget : std::uint8_t { Local, Online, Unavailable };
struct RouteDecision { RouteTarget target{RouteTarget::Unavailable}; std::string providerId,reason; };
class ProviderRouter final { public: static RouteDecision choose(bool localModelReady,bool onlineAllowed,bool networkAvailable,const ProviderRegistry&); };
std::unique_ptr<ProviderAdapter> makeProviderAdapter(ProviderKind);
bool isSafeProviderUrl(const ProviderConfig&,std::string& error);
}
