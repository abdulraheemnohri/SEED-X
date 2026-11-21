#include <jni.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include "seed/agent_kernel.hpp"
#include "seed/llama_backend.hpp"

namespace {
seed::AgentKernel gKernel;
std::mutex gMutex;
std::atomic<bool> gCancel{false};
bool gInitialized=false;
constexpr std::uint64_t kReserveBytes=768ULL*1024ULL*1024ULL;

std::string fromJava(JNIEnv* env,jstring value) {
 if(!value) return {};
 const char* chars=env->GetStringUTFChars(value,nullptr);
 if(!chars) return {};
 std::string out(chars);
 env->ReleaseStringUTFChars(value,chars);
 return out;
}
jstring toJava(JNIEnv* env,const std::string& value) { return env->NewStringUTF(value.c_str()); }
std::string stateName(seed::RuntimeState s) {
 switch(s) {
  case seed::RuntimeState::Unavailable:return "unavailable";
  case seed::RuntimeState::Unloaded:return "unloaded";
  case seed::RuntimeState::Loading:return "loading";
  case seed::RuntimeState::Ready:return "ready";
  case seed::RuntimeState::Generating:return "generating";
  case seed::RuntimeState::Error:return "error";
 }
 return "unknown";
}
std::string escapeJson(const std::string& s) {
 std::string out;
 for(unsigned char c:s) {
  if(c=='"'||c=='\\') { out+='\\'; out+=static_cast<char>(c); }
  else if(c=='\n') out+="\\n";
  else if(c=='\r') out+="\\r";
  else if(c=='\t') out+="\\t";
  else if(c>=0x20) out+=static_cast<char>(c);
 }
 return out;
}
std::string takeCompleteUtf8(std::string& pending) {
 std::string ready;
 const std::string replacement="\xEF\xBF\xBD";
 std::size_t i=0;
 while(i<pending.size()) {
  const auto a=static_cast<unsigned char>(pending[i]);
  std::size_t n=0;
  if(a<=0x7f) n=1;
  else if(a>=0xc2&&a<=0xdf) n=2;
  else if(a>=0xe0&&a<=0xef) n=3;
  else if(a>=0xf0&&a<=0xf4) n=4;
  else { ready+=replacement; ++i; continue; }
  if(i+n>pending.size()) break;
  bool valid=true;
  for(std::size_t j=1;j<n;++j) {
   const auto b=static_cast<unsigned char>(pending[i+j]);
   if((b&0xc0)!=0x80) valid=false;
  }
  if(n==3) {
   const auto b=static_cast<unsigned char>(pending[i+1]);
   if((a==0xe0&&b<0xa0)||(a==0xed&&b>=0xa0)) valid=false;
  }
  if(n==4) {
   const auto b=static_cast<unsigned char>(pending[i+1]);
   if((a==0xf0&&b<0x90)||(a==0xf4&&b>=0x90)) valid=false;
  }
  if(!valid) { ready+=replacement; ++i; continue; }
  ready.append(pending,i,n);
  i+=n;
 }
 pending.erase(0,i);
 return ready;
}
void ensureInitialized() {
 if(!gInitialized) gInitialized=gKernel.initialize();
}
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_seedx_agent_MainActivity_nativeInit(JNIEnv*,jobject) {
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 return gInitialized?JNI_TRUE:JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeRuntimeStatus(JNIEnv* env,jobject) {
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 seed::DeviceMemory mem;
 std::string memError;
 const bool hasMemory=seed::DeviceMemoryProbe::read(mem,memError,kReserveBytes);
 const auto state=gKernel.localAi().state();
 const bool ready=state==seed::RuntimeState::Ready;
 std::string json="{\"ready\":";
 json+=ready?"true":"false";
 json+=",\"state\":\""+stateName(state)+"\",\"backend\":\""+escapeJson(gKernel.localAi().backendName())+"\",\"model\":\""+escapeJson(gKernel.localAi().loadedModelId())+"\",\"llamaAvailable\":";
 json+=seed::llamaCppBackendAvailable()?"true":"false";
 json+=",\"availableBytes\":"+std::to_string(hasMemory?mem.availableBytes:0);
 json+=",\"reserveBytes\":"+std::to_string(kReserveBytes);
 json+=",\"memoryError\":\""+escapeJson(memError)+"\"}";
 return toJava(env,json);
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeLoadModel(JNIEnv* env,jobject,jstring pathValue,jstring idValue,jstring displayValue,jint contextTokens) {
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 if(gKernel.emergencyStopped()) return toJava(env,"Guardian emergency stop is active.");
 if(!seed::llamaCppBackendAvailable()) return toJava(env,"llama.cpp backend is not included. Build with -PseedxLlamaCpp=ON.");
 const std::string path=fromJava(env,pathValue);
 const std::string id=fromJava(env,idValue);
 const std::string display=fromJava(env,displayValue);
 if(path.empty()||id.empty()||display.empty()) return toJava(env,"Model path and identity are required.");
 std::error_code ec;
 const auto bytes=std::filesystem::file_size(path,ec);
 if(ec||bytes<4) return toJava(env,"Model file is missing or unreadable.");
 seed::DeviceMemory memory;
 std::string error;
 if(!seed::DeviceMemoryProbe::read(memory,error,kReserveBytes)) return toJava(env,error);
 seed::ModelManifest manifest;
 manifest.id=id;
 manifest.displayName=display;
 manifest.filePath=path;
 manifest.format=seed::ModelFormat::GGUF;
 manifest.fileBytes=bytes;
 manifest.estimatedRamBytes=bytes+std::max<std::uint64_t>(bytes/2,256ULL*1024ULL*1024ULL);
 manifest.contextTokens=static_cast<std::uint32_t>(std::clamp(contextTokens,512,8192));
 if(!gKernel.localAi().load(manifest,seed::makeLlamaCppBackend(),memory,error)) return toJava(env,error);
 return toJava(env,"Model loaded successfully.");
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeGenerate(JNIEnv* env,jobject,jstring promptValue,jint maxTokens,jfloat temperature,jfloat topP,jobject listener) {
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 gCancel.store(false);
 if(gKernel.emergencyStopped()) return toJava(env,"ERROR: Guardian emergency stop is active.");
 const std::string prompt=fromJava(env,promptValue);
 jclass listenerClass=nullptr;
 jmethodID onToken=nullptr;
 if(listener) {
  listenerClass=env->GetObjectClass(listener);
  if(listenerClass) onToken=env->GetMethodID(listenerClass,"onToken","(Ljava/lang/String;)V");
  if(env->ExceptionCheck()) { env->ExceptionClear(); onToken=nullptr; }
 }
 std::string pendingUtf8;
 seed::GenerationConfig config;
 config.maxTokens=static_cast<std::uint32_t>(std::clamp(maxTokens,1,2048));
 config.temperature=temperature;
 config.topP=topP;
 const auto result=gKernel.localAi().generate(prompt,config,[&](const std::string& chunk) {
  if(gCancel.load()) return false;
  pendingUtf8+=chunk;
  const std::string safeChunk=takeCompleteUtf8(pendingUtf8);
  if(listener&&onToken&&!safeChunk.empty()) {
   jstring text=toJava(env,safeChunk);
   env->CallVoidMethod(listener,onToken,text);
   env->DeleteLocalRef(text);
   if(env->ExceptionCheck()) { env->ExceptionClear(); return false; }
  }
  return !gCancel.load();
 });
 if(listener&&onToken&&!pendingUtf8.empty()&&!gCancel.load()) {
  jstring tail=toJava(env,"\xEF\xBF\xBD");
  env->CallVoidMethod(listener,onToken,tail);
  env->DeleteLocalRef(tail);
  if(env->ExceptionCheck()) env->ExceptionClear();
 }
 if(listenerClass) env->DeleteLocalRef(listenerClass);
 if(!result.success) return toJava(env,"ERROR: "+result.error);
 return toJava(env,result.text);
}

extern "C" JNIEXPORT void JNICALL
Java_org_seedx_agent_MainActivity_nativeCancel(JNIEnv*,jobject) { gCancel.store(true); }

extern "C" JNIEXPORT void JNICALL
Java_org_seedx_agent_MainActivity_nativeUnloadModel(JNIEnv*,jobject) {
 std::lock_guard<std::mutex> lock(gMutex);
 gKernel.localAi().unload();
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeGuardianStatus(JNIEnv* env,jobject) {
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 const bool frozen=gKernel.emergencyStopped();
 const bool audit=gKernel.verifyAudit();
 const auto status=static_cast<int>(gKernel.status());
 std::string json="{\"emergencyStop\":";
 json+=frozen?"true":"false";
 json+=",\"auditValid\":";
 json+=audit?"true":"false";
 json+=",\"kernelStatus\":"+std::to_string(status)+"}";
 return toJava(env,json);
}

extern "C" JNIEXPORT void JNICALL
Java_org_seedx_agent_MainActivity_nativeEmergencyStop(JNIEnv*,jobject) {
 gCancel.store(true);
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 gKernel.emergencyStop();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_seedx_agent_MainActivity_nativeResetEmergencyStop(JNIEnv*,jobject,jboolean confirmed) {
 if(confirmed!=JNI_TRUE) return JNI_FALSE;
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 return gKernel.resetEmergencyStop(true)?JNI_TRUE:JNI_FALSE;
}
