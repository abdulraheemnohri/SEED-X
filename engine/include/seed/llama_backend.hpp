#pragma once
#include "seed/local_ai.hpp"
#include <memory>
namespace seed {
std::unique_ptr<InferenceBackend> makeLlamaCppBackend();
bool llamaCppBackendAvailable() noexcept;
}
