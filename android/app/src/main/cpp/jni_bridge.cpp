#include <jni.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>
#include <string>
#include <vector>
#include "seed/agent_kernel.hpp"
#include "seed/llama_backend.hpp"
#include "seed/tool_registry.hpp"

namespace {
seed::AgentKernel gKernel;
seed::ToolRegistry gTools;
std::filesystem::path gWorkspaceRoot;
bool gToolsRegistered=false;
std::mutex gMutex;
std::atomic<bool> gCancel{false};
std::atomic<bool> gModelReady{false};
bool gInitialized=false;
constexpr std::uint64_t kReserveBytes=768ULL*1024ULL*1024ULL;

std::string fromJava(JNIEnv* env,jstring value) {
 if(!value) return {};
 const jsize length=env->GetStringLength(value);
 const jchar* chars=env->GetStringChars(value,nullptr);
 if(!chars) return {};
 std::string out;
 out.reserve(static_cast<std::size_t>(length)*3);
 for(jsize i=0;i<length;++i) {
  std::uint32_t cp=chars[i];
  if(cp>=0xd800&&cp<=0xdbff&&i+1<length) {
   const std::uint32_t low=chars[i+1];
   if(low>=0xdc00&&low<=0xdfff) { cp=0x10000+((cp-0xd800)<<10)+(low-0xdc00); ++i; }
   else cp=0xfffd;
  } else if(cp>=0xdc00&&cp<=0xdfff) cp=0xfffd;
  if(cp<=0x7f) out.push_back(static_cast<char>(cp));
  else if(cp<=0x7ff) { out.push_back(static_cast<char>(0xc0|(cp>>6))); out.push_back(static_cast<char>(0x80|(cp&0x3f))); }
  else if(cp<=0xffff) { out.push_back(static_cast<char>(0xe0|(cp>>12))); out.push_back(static_cast<char>(0x80|((cp>>6)&0x3f))); out.push_back(static_cast<char>(0x80|(cp&0x3f))); }
  else { out.push_back(static_cast<char>(0xf0|(cp>>18))); out.push_back(static_cast<char>(0x80|((cp>>12)&0x3f))); out.push_back(static_cast<char>(0x80|((cp>>6)&0x3f))); out.push_back(static_cast<char>(0x80|(cp&0x3f))); }
 }
 env->ReleaseStringChars(value,chars);
 return out;
}
jstring toJava(JNIEnv* env,const std::string& value) {
 std::vector<jchar> utf16;
 utf16.reserve(value.size());
 const auto* bytes=reinterpret_cast<const unsigned char*>(value.data());
 std::size_t i=0;
 while(i<value.size()) {
  const unsigned char a=bytes[i];
  std::uint32_t cp=0xfffd;
  std::size_t n=1;
  if(a<=0x7f) cp=a;
  else if(a>=0xc2&&a<=0xdf&&i+1<value.size()&&(bytes[i+1]&0xc0)==0x80) { cp=((a&0x1f)<<6)|(bytes[i+1]&0x3f); n=2; }
  else if(a>=0xe0&&a<=0xef&&i+2<value.size()&&(bytes[i+1]&0xc0)==0x80&&(bytes[i+2]&0xc0)==0x80) {
   cp=((a&0x0f)<<12)|((bytes[i+1]&0x3f)<<6)|(bytes[i+2]&0x3f); n=3;
   if(cp<0x800||(cp>=0xd800&&cp<=0xdfff)) cp=0xfffd;
  } else if(a>=0xf0&&a<=0xf4&&i+3<value.size()&&(bytes[i+1]&0xc0)==0x80&&(bytes[i+2]&0xc0)==0x80&&(bytes[i+3]&0xc0)==0x80) {
   cp=((a&0x07)<<18)|((bytes[i+1]&0x3f)<<12)|((bytes[i+2]&0x3f)<<6)|(bytes[i+3]&0x3f); n=4;
   if(cp<0x10000||cp>0x10ffff) cp=0xfffd;
  }
  if(cp<=0xffff) utf16.push_back(static_cast<jchar>(cp));
  else { cp-=0x10000; utf16.push_back(static_cast<jchar>(0xd800+(cp>>10))); utf16.push_back(static_cast<jchar>(0xdc00+(cp&0x3ff))); }
  i+=n;
 }
 return env->NewString(utf16.data(),static_cast<jsize>(utf16.size()));
}
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

bool withinRoot(const std::filesystem::path& candidate,const std::filesystem::path& root) {
 const auto rel=candidate.lexically_relative(root);
 if(rel.empty()) return candidate==root;
 return !rel.is_absolute() && *rel.begin()!="..";
}
bool relativeResource(const std::string& resource,std::string& relative) {
 const std::string prefix="workspace://";
 if(resource.size()>512||resource.rfind(prefix,0)!=0) return false;
 relative=resource.substr(prefix.size());
 if(relative.size()>480||relative.find('\\')!=std::string::npos) return false;
 for(unsigned char ch:relative) if(ch==0||ch<0x20) return false;
 std::filesystem::path p(relative);
 if(p.is_absolute()) return false;
 for(const auto& part:p) if(part==".."||part==".") return false;
 return true;
}
void registerTools(const std::filesystem::path& root) {
 if(gToolsRegistered) return;
 gWorkspaceRoot=std::filesystem::weakly_canonical(root);
 gTools.add({"system.status","read","system://","Read SEED-X runtime status"},
  [](const seed::ToolRequest&,const std::string&) {
   return seed::ToolResult{true,seed::Decision::Allow,
    []{
     seed::DeviceMemory memory;std::string error;
     const bool available=seed::DeviceMemoryProbe::read(memory,error,kReserveBytes);
     return std::string("{\"kernelReady\":true,\"emergencyStop\":")+
      (gKernel.emergencyStopped()?"true":"false")+",\"modelReady\":"+
      (gModelReady.load()?"true":"false")+",\"availableBytes\":"+
      std::to_string(available?memory.availableBytes:0)+",\"memoryProbeOk\":"+
      (available?"true":"false")+"}";
    }()};
  });
 gTools.add({"files.list","read","workspace://","List files in the private SEED-X workspace"},
  [root=gWorkspaceRoot](const seed::ToolRequest& request,const std::string&) {
   std::string rel;if(!relativeResource(request.resource,rel))return seed::ToolResult{false,seed::Decision::Deny,"Invalid workspace path."};
   std::error_code ec;auto target=std::filesystem::canonical(root/std::filesystem::path(rel),ec);
   if(ec||!withinRoot(target,root)||!std::filesystem::is_directory(target))return seed::ToolResult{false,seed::Decision::Deny,"Workspace directory does not exist or is outside the sandbox."};
   std::string json="[";std::size_t count=0;
   for(const auto& item:std::filesystem::directory_iterator(target,ec)) {
    if(ec||count>=500)break;
    if(item.is_symlink(ec))continue;
    if(count++)json+=",";
    const auto name=item.path().filename().string();
    json+="{\"name\":\""+escapeJson(name)+"\",\"type\":\""+(item.is_directory(ec)?std::string("directory"):std::string("file"))+"\",\"bytes\":"+std::to_string(item.is_regular_file(ec)?item.file_size(ec):0)+"}";
   }
   json+="]";return seed::ToolResult{true,seed::Decision::Allow,json};
  });
 gTools.add({"files.read","read","workspace://","Read a small text file from the private workspace"},
  [root=gWorkspaceRoot](const seed::ToolRequest& request,const std::string&) {
   std::string rel;if(!relativeResource(request.resource,rel)||rel.empty())return seed::ToolResult{false,seed::Decision::Deny,"A relative workspace file path is required."};
   std::error_code ec;auto target=std::filesystem::canonical(root/std::filesystem::path(rel),ec);
   if(ec||!withinRoot(target,root)||!std::filesystem::is_regular_file(target))return seed::ToolResult{false,seed::Decision::Deny,"File does not exist or is outside the sandbox."};
   const auto size=std::filesystem::file_size(target,ec);if(ec||size>1024*1024)return seed::ToolResult{false,seed::Decision::Deny,"File exceeds the 1 MiB read limit."};
   std::ifstream in(target,std::ios::binary);if(!in)return seed::ToolResult{false,seed::Decision::Deny,"Cannot open workspace file."};
   std::ostringstream data;data<<in.rdbuf();return seed::ToolResult{true,seed::Decision::Allow,data.str()};
  });
 gTools.add({"files.write","create","workspace://","Create a new note after one-use operator approval"},
  [root=gWorkspaceRoot](const seed::ToolRequest& request,const std::string& payload) {
   std::string rel;if(!relativeResource(request.resource,rel)||rel.empty())return seed::ToolResult{false,seed::Decision::Deny,"A relative workspace file path is required."};
   if(payload.empty()||payload.size()>65536)return seed::ToolResult{false,seed::Decision::Deny,"Note must contain 1 to 65536 bytes."};
   std::error_code ec;const auto parent=std::filesystem::canonical((root/std::filesystem::path(rel)).parent_path(),ec);
   if(ec||!withinRoot(parent,root)||!std::filesystem::is_directory(parent))return seed::ToolResult{false,seed::Decision::Deny,"Parent directory must already exist inside workspace."};
   const auto target=parent/std::filesystem::path(rel).filename();
   if(std::filesystem::exists(target,ec)||ec)return seed::ToolResult{false,seed::Decision::Deny,"Refusing to overwrite an existing file."};
   const int fd=::open(target.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0600);
   if(fd<0)return seed::ToolResult{false,seed::Decision::Deny,errno==EEXIST?"Refusing to overwrite an existing file.":"Cannot create workspace note."};
   std::size_t offset=0;bool writeOk=true;
   while(offset<payload.size()) {
    const auto n=::write(fd,payload.data()+offset,payload.size()-offset);
    if(n<0&&errno==EINTR)continue;
    if(n<=0){writeOk=false;break;}
    offset+=static_cast<std::size_t>(n);
   }
   const bool closeOk=::close(fd)==0;
   if(!writeOk||!closeOk){::unlink(target.c_str());return seed::ToolResult{false,seed::Decision::Deny,"Workspace note write failed; partial file removed."};}
   return seed::ToolResult{true,seed::Decision::Allow,"Created "+target.filename().string()};
  });
 gTools.add({"workspace.search","search","workspace://","Search text files in the private workspace"},
  [root=gWorkspaceRoot](const seed::ToolRequest& request,const std::string& payload) {
   std::string rel;
   if(!relativeResource(request.resource,rel))return seed::ToolResult{false,seed::Decision::Deny,"Invalid workspace search path."};
   if(payload.empty()||payload.size()>256)return seed::ToolResult{false,seed::Decision::Deny,"Search query must contain 1 to 256 bytes."};
   auto lower=[](std::string value){for(char& ch:value)ch=static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));return value;};
   const std::string query=lower(payload);
   std::error_code ec;const auto start=std::filesystem::canonical(root/std::filesystem::path(rel),ec);
   if(ec||!withinRoot(start,root)||!std::filesystem::is_directory(start))return seed::ToolResult{false,seed::Decision::Deny,"Search directory does not exist or is outside workspace."};
   std::string json="[";std::size_t matches=0,visited=0;
   for(std::filesystem::recursive_directory_iterator it(start,std::filesystem::directory_options::skip_permission_denied,ec),end;it!=end&&visited<200&&matches<50;it.increment(ec)){
    if(ec){ec.clear();continue;}
    if(it.depth()>3){it.disable_recursion_pending();continue;}
    if(it->is_symlink(ec)){it.disable_recursion_pending();continue;}
    if(!it->is_regular_file(ec))continue;
    ++visited;const auto size=it->file_size(ec);if(ec||size>262144){ec.clear();continue;}
    std::ifstream in(it->path(),std::ios::binary);if(!in)continue;
    std::string data((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
    if(data.find('\0')!=std::string::npos)continue;
    if(lower(data).find(query)==std::string::npos)continue;
    const auto relPath=it->path().lexically_relative(root).generic_string();
    if(matches++)json+=",";
    json+="{\\"path\\":\\""+escapeJson(relPath)+"\\",\\"bytes\\":"+std::to_string(size)+"}";
   }
   json+="]";
   return seed::ToolResult{true,seed::Decision::Allow,json};
  });
 gToolsRegistered=true;
}

void ensureInitialized() {
 if(!gInitialized) gInitialized=gKernel.initialize();
}
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_seedx_agent_MainActivity_nativeInit(JNIEnv* env,jobject,jstring workspaceValue) {
 std::lock_guard<std::mutex> lock(gMutex);
 ensureInitialized();
 const std::string base=fromJava(env,workspaceValue);
 if(base.empty()) return JNI_FALSE;
 std::error_code ec;auto workspace=std::filesystem::path(base)/"workspace";
 std::filesystem::create_directories(workspace,ec);
 if(ec) return JNI_FALSE;
 registerTools(workspace);
 return gInitialized&&gToolsRegistered?JNI_TRUE:JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeRuntimeStatus(JNIEnv* env,jobject) {
 std::unique_lock<std::mutex> lock(gMutex,std::try_to_lock);
 ensureInitialized();
 if(!lock.owns_lock()) {
  std::string busy="{\"ready\":";
  busy+=gModelReady.load()?"true":"false";
  busy+=",\"state\":\"busy\",\"llamaAvailable\":";
  busy+=seed::llamaCppBackendAvailable()?"true":"false";
  busy+=",\"availableBytes\":0,\"reserveBytes\":"+std::to_string(kReserveBytes)+"}";
  return toJava(env,busy);
 }
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
 const bool loaded=gKernel.localAi().load(manifest,seed::makeLlamaCppBackend(),memory,error);
 gModelReady.store(gKernel.localAi().state()==seed::RuntimeState::Ready);
 if(!loaded) return toJava(env,error);
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
 gModelReady.store(false);
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeGuardianStatus(JNIEnv* env,jobject) {
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
 // Guardian has its own internal synchronization. Do not wait for the inference mutex:
 // emergency stop must be able to interrupt a generation callback immediately.
 gKernel.emergencyStop();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_seedx_agent_MainActivity_nativeResetEmergencyStop(JNIEnv*,jobject,jboolean confirmed) {
 if(confirmed!=JNI_TRUE) return JNI_FALSE;
 return gKernel.resetEmergencyStop(true)?JNI_TRUE:JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeToolCatalog(JNIEnv* env,jobject) {
 return toJava(env,"[{\"tool\":\"system.status\",\"operation\":\"read\",\"resource\":\"system://status\",\"description\":\"Read device and runtime status\"},{\"tool\":\"files.list\",\"operation\":\"read\",\"resource\":\"workspace://\",\"description\":\"List private workspace files\"},{\"tool\":\"files.read\",\"operation\":\"read\",\"resource\":\"workspace://relative/path.txt\",\"description\":\"Read a workspace text file up to 1 MiB\"},{\"tool\":\"files.write\",\"operation\":\"create\",\"resource\":\"workspace://notes/new.txt\",\"description\":\"Create a new note; operator approval required\"}]");
}
extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeRequestApproval(JNIEnv* env,jobject,jstring toolValue,jstring operationValue,jstring resourceValue,jboolean confirmed) {
 if(confirmed!=JNI_TRUE||gKernel.emergencyStopped()) return toJava(env,"");
 const std::string tool=fromJava(env,toolValue),operation=fromJava(env,operationValue),resource=fromJava(env,resourceValue);
 if(tool.size()>128||operation.size()>64||resource.size()>512)return toJava(env,"");
 seed::ToolRequest request{tool,operation,resource,"Operator-approved Phase 7 tool request"};
 const auto ticket=gKernel.requestApproval(request,true);
 return toJava(env,ticket.id);
}
extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeExecuteTool(JNIEnv* env,jobject,jstring toolValue,jstring operationValue,jstring resourceValue,jstring payloadValue,jstring ticketValue) {
 if(gKernel.emergencyStopped()) return toJava(env,"{\"success\":false,\"decision\":\"deny\",\"output\":\"Guardian emergency stop is active.\"}");
 seed::ToolRequest request{fromJava(env,toolValue),fromJava(env,operationValue),fromJava(env,resourceValue),"SEED-X Android Tools screen request"};
 const auto result=gTools.execute(gKernel,request,fromJava(env,payloadValue),fromJava(env,ticketValue));
 const std::string decision=result.decision==seed::Decision::Allow?"allow":result.decision==seed::Decision::RequireApproval?"approval":"deny";
 return toJava(env,std::string("{\"success\":")+(result.success?"true":"false")+",\"decision\":\""+decision+"\",\"output\":\""+escapeJson(result.output)+"\"}");
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativePolicySnapshot(JNIEnv* env,jobject) {
 const auto rules=gKernel.policies();std::string json="[";
 for(std::size_t i=0;i<rules.size();++i){const auto& r=rules[i];if(i)json+=",";
  const char* effect=r.effect==seed::PolicyEffect::Allow?"allow":r.effect==seed::PolicyEffect::RequireApproval?"approval":"deny";
  json+="{\"tool\":\""+escapeJson(r.tool)+"\",\"operation\":\""+escapeJson(r.operation)+"\",\"resourcePrefix\":\""+escapeJson(r.resourcePrefix)+"\",\"effect\":\""+effect+"\",\"enabled\":"+(r.enabled?"true":"false")+"}";
 }
 json+="]";return toJava(env,json);
}
extern "C" JNIEXPORT jstring JNICALL
Java_org_seedx_agent_MainActivity_nativeAuditLog(JNIEnv* env,jobject) {
 const auto entries=gKernel.auditSnapshot();const std::size_t begin=entries.size()>100?entries.size()-100:0;
 std::string json="[";
 for(std::size_t i=begin;i<entries.size();++i){const auto& e=entries[i];if(i>begin)json+=",";
  const char* decision=e.decision==seed::Decision::Allow?"allow":e.decision==seed::Decision::RequireApproval?"approval":"deny";
  json+="{\"sequence\":"+std::to_string(e.sequence)+",\"timestampMs\":"+std::to_string(e.timestampMs)+",\"tool\":\""+escapeJson(e.tool)+"\",\"operation\":\""+escapeJson(e.operation)+"\",\"resource\":\""+escapeJson(e.resource)+"\",\"decision\":\""+decision+"\",\"reason\":\""+escapeJson(e.reason)+"\"}";
 }
 json+="]";return toJava(env,json);
}
