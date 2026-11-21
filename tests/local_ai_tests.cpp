#include "seed/local_ai.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
class TestBackend final:public seed::InferenceBackend{
public:
 bool loaded{false};
 std::string name()const override{return "test-backend";}
 bool supports(seed::ModelFormat f)const override{return f==seed::ModelFormat::GGUF;}
 bool load(const seed::ModelManifest&,std::string&)override{loaded=true;return true;}
 void unload()noexcept override{loaded=false;}
 seed::GenerationResult generate(const std::string& prompt,const seed::GenerationConfig&,const seed::TokenCallback& cb)override{
  const std::string output="Test response: "+prompt;
  if(cb&&!cb(output))return {false,{},"Generation cancelled by callback."};
  return {true,output,{}};
 }
};
int main(){
 const auto file=std::filesystem::temp_directory_path()/"seedx-test-model.gguf";
 {std::ofstream out(file,std::ios::binary);out<<"SEEDX_TEST_MODEL";}
 seed::ModelManifest model{"test-model","Test model",file.string(),seed::ModelFormat::GGUF,std::filesystem::file_size(file),256ULL*1024*1024,2048,{}};
 assert(seed::ModelManager::validate(model).valid);
 assert(!seed::ModelManager::fitsMemory(model,{512ULL*1024*1024,512ULL*1024*1024}));
 assert(seed::ModelManager::fitsMemory(model,{1024ULL*1024*1024,512ULL*1024*1024}));
 auto candidates=seed::ModelManager::fitCandidates({model},{1024ULL*1024*1024,512ULL*1024*1024});
 assert(candidates.size()==1&&candidates[0].id=="test-model");
 seed::LocalAiRuntime runtime;std::string error;
 assert(runtime.load(model,std::make_unique<TestBackend>(),{1024ULL*1024*1024,512ULL*1024*1024},error));
 assert(runtime.state()==seed::RuntimeState::Ready&&runtime.backendName()=="test-backend");
 std::vector<std::string> streamed;
 auto result=runtime.generate("hello",{32,0.7F,0.9F},[&](const std::string& token){streamed.push_back(token);return true;});
 assert(result.success&&result.text=="Test response: hello"&&streamed.size()==1);
 auto bad=runtime.generate("hello",{0,0.7F,0.9F});assert(!bad.success);
 auto cancelled=runtime.generate("hello",{32,0.7F,0.9F},[](const std::string&){return false;});assert(!cancelled.success);
 runtime.unload();assert(runtime.state()==seed::RuntimeState::Unavailable);
 std::error_code ec;std::filesystem::remove(file,ec);
 std::cout<<"SEED-X Phase 3 runtime tests passed\n";
}
