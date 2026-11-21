#include "seed/runtime_registry.hpp"
#include "seed/llama_backend.hpp"
#include <cassert>
#include <iostream>
class Dummy final:public seed::InferenceBackend{
public:std::string name()const override{return "dummy";}bool supports(seed::ModelFormat f)const override{return f==seed::ModelFormat::GGUF;}
 bool load(const seed::ModelManifest&,std::string&)override{return true;}void unload()noexcept override{}
 seed::GenerationResult generate(const std::string&,const seed::GenerationConfig&,const seed::TokenCallback&)override{return {true,"ok",{}};}
};
int main(){
 seed::RuntimeRegistry r;
 assert(!r.add({"","Invalid","embedded",{},seed::RuntimeAvailability::Available,true,""},[]{return std::make_unique<Dummy>();}));
 assert(!r.add({"fake","Fake","embedded",{},seed::RuntimeAvailability::Available,true,""}));
 assert(r.add({"fake","Fake","embedded",{seed::ModelFormat::GGUF},seed::RuntimeAvailability::Available,true,"test"},[]{return std::make_unique<Dummy>();}));
 assert(!r.add({"fake","Duplicate","embedded",{},seed::RuntimeAvailability::AdapterMissing,true,""}));
 assert(r.canRun("fake"));auto backend=r.create("fake");assert(backend&&backend->name()=="dummy");
 seed::RuntimeRegistry mismatch;assert(mismatch.add({"bad-factory","Bad factory","embedded",{seed::ModelFormat::ONNX},seed::RuntimeAvailability::Available,true,"test"},[]{return std::make_unique<Dummy>();}));assert(!mismatch.create("bad-factory"));
 assert(r.add({"planned","Planned","embedded",{},seed::RuntimeAvailability::AdapterMissing,true,"not integrated"}));
 assert(!r.canRun("planned")&&!r.create("planned"));assert(!r.canRun("missing"));
 auto builtins=seed::RuntimeRegistry::builtins();assert(builtins.find("llama.cpp"));
 assert(builtins.find("LiteRT-LM")&&!builtins.canRun("LiteRT-LM"));
 assert(builtins.find("Ollama")&&!builtins.canRun("Ollama"));
 const auto llama=builtins.find("llama.cpp");
 assert(llama->descriptor.availability==(seed::llamaCppBackendAvailable()?seed::RuntimeAvailability::Available:seed::RuntimeAvailability::OptionalBuildDisabled));
 std::cout<<"SEED-X runtime registry tests passed
";
}
