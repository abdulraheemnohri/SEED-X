#include "seed/llama_backend.hpp"
#include "llama.h"
#include <algorithm>
#include <climits>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
namespace seed {
namespace {
std::once_flag backendInitFlag;
class LlamaBackend final:public InferenceBackend {
public:
 ~LlamaBackend()override{unload();}
 std::string name()const override{return "llama.cpp";}
 bool supports(ModelFormat f)const override{return f==ModelFormat::GGUF;}
 bool load(const ModelManifest&m,std::string&error)override{
  unload();std::call_once(backendInitFlag,[]{llama_backend_init();});
  auto mp=llama_model_default_params();mp.use_mmap=true;mp.n_gpu_layers=0;
  model_=llama_model_load_from_file(m.filePath.c_str(),mp);
  if(!model_){error="llama.cpp could not load this GGUF model.";return false;}
  auto cp=llama_context_default_params();cp.n_ctx=m.contextTokens;cp.n_batch=std::min<std::uint32_t>(m.contextTokens,512);
  const auto hw=std::thread::hardware_concurrency();const auto threads=static_cast<int32_t>(std::max(1U,hw>1?hw-1:1U));cp.n_threads=threads;cp.n_threads_batch=threads;
  context_=llama_init_from_model(model_,cp);
  if(!context_){llama_model_free(model_);model_=nullptr;error="llama.cpp could not create an inference context.";return false;}
  vocab_=llama_model_get_vocab(model_);contextLimit_=m.contextTokens;error.clear();return true;
 }
 void unload()noexcept override{if(context_){llama_free(context_);context_=nullptr;}if(model_){llama_model_free(model_);model_=nullptr;}vocab_=nullptr;contextLimit_=0;}
 GenerationResult generate(const std::string&prompt,const GenerationConfig&cfg,const TokenCallback&callback)override{
  if(!context_||!model_||!vocab_)return {false,{},"llama.cpp model is not loaded."};
  llama_kv_cache_clear(context_);
  if(prompt.size()>static_cast<std::size_t>(INT32_MAX))return {false,{},"Prompt is too large to tokenize."};
  const int32_t needed=-llama_tokenize(vocab_,prompt.c_str(),static_cast<int32_t>(prompt.size()),nullptr,0,true,false);
  if(needed<=0)return {false,{},"Prompt tokenization failed."};
  if(static_cast<std::uint32_t>(needed)>=contextLimit_)return {false,{},"Prompt leaves no room for generated tokens."};
  std::vector<llama_token>tokens(static_cast<std::size_t>(needed));
  const int32_t count=llama_tokenize(vocab_,prompt.c_str(),static_cast<int32_t>(prompt.size()),tokens.data(),needed,true,false);
  if(count<0)return {false,{},"Prompt tokenization failed."};
  struct BatchGuard{llama_batch value;~BatchGuard(){llama_batch_free(value);}};
  BatchGuard batch{llama_batch_init(std::max(1,count),0,1)};
  if(!batch.value.token||!batch.value.pos||!batch.value.n_seq_id||!batch.value.seq_id||!batch.value.logits)return {false,{},"Unable to allocate prompt batch."};
  batch.value.n_tokens=count;
  for(int32_t i=0;i<count;++i){batch.value.token[i]=tokens[static_cast<std::size_t>(i)];batch.value.pos[i]=i;batch.value.n_seq_id[i]=1;batch.value.seq_id[i][0]=0;batch.value.logits[i]=(i==count-1);}
  if(llama_decode(context_,batch.value)!=0)return {false,{},"llama.cpp failed to evaluate the prompt."};
  auto*chain=llama_sampler_chain_init(llama_sampler_chain_default_params());if(!chain)return {false,{},"Unable to initialize sampler."};
  struct SamplerGuard{llama_sampler*p;~SamplerGuard(){if(p)llama_sampler_free(p);}}sampler{chain};
  if(cfg.temperature<=0.0001F)llama_sampler_chain_add(chain,llama_sampler_init_greedy());
  else{llama_sampler_chain_add(chain,llama_sampler_init_top_p(cfg.topP,1));llama_sampler_chain_add(chain,llama_sampler_init_temp(cfg.temperature));llama_sampler_chain_add(chain,llama_sampler_init_dist(LLAMA_DEFAULT_SEED));}
  llama_batch next=llama_batch_init(1,0,1);struct NextGuard{llama_batch value;~NextGuard(){llama_batch_free(value);}}nextGuard{next};
  if(!next.token||!next.pos||!next.n_seq_id||!next.seq_id||!next.logits)return {false,{},"Unable to allocate generation batch."};
  std::string output;
  for(std::uint32_t step=0;step<cfg.maxTokens;++step){
   const llama_token token=llama_sampler_sample(chain,context_,-1);llama_sampler_accept(chain,token);
   if(llama_vocab_is_eog(vocab_,token))break;
   char piece[1024];const int32_t n=llama_token_to_piece(vocab_,token,piece,sizeof(piece),0,false);
   if(n<0)return {false,output,"Token-to-text conversion failed."};
   std::string chunk(piece,static_cast<std::size_t>(n));output+=chunk;
   if(callback&&!callback(chunk))return {false,output,"Generation cancelled by callback."};
   if(step+1>=cfg.maxTokens)break;
   next.n_tokens=1;next.token[0]=token;next.pos[0]=count+static_cast<llama_pos>(step);next.n_seq_id[0]=1;next.seq_id[0][0]=0;next.logits[0]=true;
   if(llama_decode(context_,next)!=0)return {false,output,"llama.cpp failed during token generation."};
  }
  return {true,std::move(output),{}};
 }
private:llama_model*model_{nullptr};llama_context*context_{nullptr};const llama_vocab*vocab_{nullptr};std::uint32_t contextLimit_{0};
};
}
std::unique_ptr<InferenceBackend>makeLlamaCppBackend(){return std::make_unique<LlamaBackend>();}
bool llamaCppBackendAvailable()noexcept{return true;}
}
