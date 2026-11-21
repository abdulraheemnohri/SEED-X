#include "seed/llama_backend.hpp"
namespace seed {
std::unique_ptr<InferenceBackend> makeLlamaCppBackend(){return {};}
bool llamaCppBackendAvailable() noexcept{return false;}
}
