#include "seed/local_ai.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <utility>
namespace seed {
ModelCheck ModelManager::validate(const ModelManifest&m){
 if(m.id.empty()||m.displayName.empty()||m.filePath.empty())return {false,"Model id, display name and file path are required."};
 if(m.id.size()>128||m.displayName.size()>256||m.filePath.size()>2048)return {false,"Model metadata exceeds allowed limits."};
 if(m.format!=ModelFormat::GGUF)return {false,"Only GGUF manifests are currently accepted by this runtime contract."};
 if(m.contextTokens<128||m.contextTokens>131072)return {false,"Context length must be between 128 and 131072 tokens."};
 if(m.estimatedRamBytes==0)return {false,"Estimated RAM requirement must be provided."};
 std::error_code ec;const auto path=std::filesystem::path(m.filePath);
 if(!std::filesystem::exists(path,ec)||ec)return {false,"Model file does not exist or cannot be accessed."};
 if(!std::filesystem::is_regular_file(path,ec)||ec)return {false,"Model path must reference a regular file."};
 const auto actual=std::filesystem::file_size(path,ec);if(ec)return {false,"Unable to read model file size."};
 if(actual==0)return {false,"Model file is empty."};
 if(m.fileBytes!=0&&m.fileBytes!=actual)return {false,"Manifest file size does not match the model file."};
 return {true,"Model manifest and file validated. Hash verification is not yet implemented."};
}
bool ModelManager::fitsMemory(const ModelManifest&m,const DeviceMemory&d){if(d.availableBytes<=d.reserveBytes)return false;const auto usable=d.availableBytes-d.reserveBytes;return m.estimatedRamBytes<=usable;}
std::vector<ModelManifest> ModelManager::fitCandidates(const std::vector<ModelManifest>&models,const DeviceMemory&d){std::vector<ModelManifest> out;for(const auto&m:models)if(validate(m).valid&&fitsMemory(m,d))out.push_back(m);std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.estimatedRamBytes<b.estimatedRamBytes;});return out;}
LocalAiRuntime::~LocalAiRuntime(){unload();}
bool LocalAiRuntime::load(const ModelManifest&m,std::unique_ptr<InferenceBackend>b,const DeviceMemory&d,std::string&error){
 if(!b){error="An inference backend is required.";return false;}
 const auto check=ModelManager::validate(m);if(!check.valid){error=check.message;return false;}
 if(!ModelManager::fitsMemory(m,d)){error="Model exceeds the configured memory budget.";return false;}
 if(!b->supports(m.format)){error="Selected backend does not support this model format.";return false;}
 unload();state_=RuntimeState::Loading;
 if(!b->load(m,error)){b->unload();state_=RuntimeState::Error;return false;}
 model_=m;backend_=std::move(b);state_=RuntimeState::Ready;error.clear();return true;
}
void LocalAiRuntime::unload()noexcept{if(backend_)backend_->unload();backend_.reset();model_=ModelManifest{};state_=RuntimeState::Unavailable;}
GenerationResult LocalAiRuntime::generate(const std::string&prompt,const GenerationConfig&config,const TokenCallback&callback){
 if(!backend_||state_!=RuntimeState::Ready)return {false,{},"No model is loaded."};
 if(prompt.empty())return {false,{},"Prompt cannot be empty."};
 if(prompt.size()>1024*1024)return {false,{},"Prompt exceeds the 1 MiB safety limit."};
 if(config.maxTokens==0||config.maxTokens>model_.contextTokens)return {false,{},"maxTokens is outside the model context limit."};
 if(!std::isfinite(config.temperature)||config.temperature<0.0F||config.temperature>2.0F)return {false,{},"Temperature must be between 0 and 2."};
 if(!std::isfinite(config.topP)||config.topP<=0.0F||config.topP>1.0F)return {false,{},"topP must be greater than 0 and at most 1."};
 state_=RuntimeState::Generating;auto result=backend_->generate(prompt,config,callback);state_=backend_?RuntimeState::Ready:RuntimeState::Unavailable;return result;
}
RuntimeState LocalAiRuntime::state()const noexcept{return state_;}
std::string LocalAiRuntime::backendName()const{return backend_?backend_->name():std::string{};}
std::string LocalAiRuntime::loadedModelId()const{return backend_?model_.id:std::string{};}
}
