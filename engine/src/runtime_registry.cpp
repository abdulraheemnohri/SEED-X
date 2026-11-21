#include "seed/runtime_registry.hpp"
#include "seed/llama_backend.hpp"
#include <algorithm>
#include <utility>
namespace seed {
bool RuntimeRegistry::add(RuntimeDescriptor d,RuntimeFactory f){
 if(d.id.empty()||d.id.size()>128||d.displayName.empty()||d.displayName.size()>160)return false;
 if(find(d.id))return false;
 if(d.availability==RuntimeAvailability::Available&&!f)return false;
 if(d.availability!=RuntimeAvailability::Available&&f)return false;
 entries_.push_back({std::move(d),std::move(f)});return true;
}
const RuntimeRegistration* RuntimeRegistry::find(const std::string&id)const noexcept{
 const auto it=std::find_if(entries_.begin(),entries_.end(),[&](const auto&e){return e.descriptor.id==id;});
 return it==entries_.end()?nullptr:&*it;
}
std::vector<RuntimeDescriptor> RuntimeRegistry::snapshot()const{
 std::vector<RuntimeDescriptor> out;out.reserve(entries_.size());for(const auto&e:entries_)out.push_back(e.descriptor);return out;
}
std::unique_ptr<InferenceBackend> RuntimeRegistry::create(const std::string&id)const{
 const auto*entry=find(id);if(!entry||entry->descriptor.availability!=RuntimeAvailability::Available||!entry->factory)return {};
 return entry->factory();
}
bool RuntimeRegistry::canRun(const std::string&id)const noexcept{
 const auto*entry=find(id);return entry&&entry->descriptor.availability==RuntimeAvailability::Available&&static_cast<bool>(entry->factory);
}
RuntimeRegistry RuntimeRegistry::builtins(){
 RuntimeRegistry r;
 if(llamaCppBackendAvailable())r.add({"llama.cpp","llama.cpp","embedded-native",{ModelFormat::GGUF},RuntimeAvailability::Available,true,"Embedded GGUF backend compiled into this build."},[]{return makeLlamaCppBackend();});
 else r.add({"llama.cpp","llama.cpp","embedded-native",{ModelFormat::GGUF},RuntimeAvailability::OptionalBuildDisabled,true,"Build with SEEDX_ENABLE_LLAMA_CPP=ON to include the embedded backend."});
 r.add({"LiteRT-LM","LiteRT-LM","embedded-mobile",{},RuntimeAvailability::AdapterMissing,true,"Catalog entry only. SEED-X adapter is not integrated."});
 r.add({"MLC-LLM","MLC LLM","embedded-mobile",{},RuntimeAvailability::AdapterMissing,true,"Catalog entry only. SEED-X adapter is not integrated."});
 r.add({"ExecuTorch","ExecuTorch","embedded-mobile",{},RuntimeAvailability::AdapterMissing,true,"Catalog entry only. SEED-X adapter is not integrated."});
 r.add({"ONNX Runtime GenAI","ONNX Runtime GenAI","embedded",{},RuntimeAvailability::AdapterMissing,true,"Catalog entry only. SEED-X adapter is not integrated."});
 r.add({"MNN-LLM","MNN-LLM","embedded-mobile",{},RuntimeAvailability::AdapterMissing,true,"Catalog entry only. SEED-X adapter is not integrated."});
 r.add({"Ollama","Ollama","remote-local-server",{},RuntimeAvailability::RemoteService,false,"Connect through the provider endpoint; not an embedded backend."});
 r.add({"LM Studio","LM Studio","remote-local-server",{},RuntimeAvailability::RemoteService,false,"Connect through the provider endpoint; not an embedded backend."});
 return r;
}
}
