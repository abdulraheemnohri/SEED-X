#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace seed {
enum class ModelFormat : std::uint8_t { GGUF, Unknown };
enum class RuntimeState : std::uint8_t { Unavailable, Unloaded, Loading, Ready, Generating, Error };
struct ModelManifest {std::string id,displayName,filePath;ModelFormat format{ModelFormat::Unknown};std::uint64_t fileBytes{0},estimatedRamBytes{0};std::uint32_t contextTokens{2048};std::string sha256;};
struct DeviceMemory {std::uint64_t availableBytes{0};std::uint64_t reserveBytes{512ULL*1024*1024};};
struct ModelCheck {bool valid{false};std::string message;};
struct GenerationConfig {std::uint32_t maxTokens{256};float temperature{0.7F};float topP{0.9F};};
struct GenerationResult {bool success{false};std::string text,error;};
using TokenCallback=std::function<bool(const std::string&)>;
class InferenceBackend {public:virtual ~InferenceBackend()=default;virtual std::string name()const=0;virtual bool supports(ModelFormat)const=0;virtual bool load(const ModelManifest&,std::string& error)=0;virtual void unload()noexcept=0;virtual GenerationResult generate(const std::string&,const GenerationConfig&,const TokenCallback&)=0;};
class ModelManager final {public:static ModelCheck validate(const ModelManifest&);static bool fitsMemory(const ModelManifest&,const DeviceMemory&);static std::vector<ModelManifest> fitCandidates(const std::vector<ModelManifest>&,const DeviceMemory&);};
class DeviceMemoryProbe final {public:static bool read(DeviceMemory& out,std::string& error,std::uint64_t reserveBytes=512ULL*1024*1024);};
class LocalAiRuntime final {
public:
 LocalAiRuntime()=default;~LocalAiRuntime();LocalAiRuntime(const LocalAiRuntime&)=delete;LocalAiRuntime&operator=(const LocalAiRuntime&)=delete;
 bool load(const ModelManifest&,std::unique_ptr<InferenceBackend>,const DeviceMemory&,std::string& error);void unload()noexcept;
 GenerationResult generate(const std::string&,const GenerationConfig&,const TokenCallback& callback={});
 RuntimeState state()const noexcept;std::string backendName()const;std::string loadedModelId()const;
private:std::unique_ptr<InferenceBackend> backend_;ModelManifest model_;RuntimeState state_{RuntimeState::Unavailable};
};
}
