# SEED-X Phase 3 — Local AI Brain

## Delivered
- Backend-neutral C++20 InferenceBackend contract.
- LocalAiRuntime load, generate, streaming callback, cancellation and unload lifecycle.
- ModelManager validates local regular files, GGUF magic, manifest size, context limits and RAM estimates.
- DeviceMemoryProbe reads Linux/Android /proc/meminfo and reports MemAvailable (falling back to MemTotal).
- RAM candidate filtering and a configurable reserve budget.
- Optional real llama.cpp backend, pinned to upstream b4514, with GGUF model loading, prompt tokenization, KV cache reset, autoregressive decoding and temperature/top-p sampling.
- Default no-backend stub keeps CI and shell builds independent of large third-party dependencies.
- Deterministic host tests validate lifecycle, memory admission, streaming, cancellation and malformed GGUF rejection.

## Build configuration
SEEDX_ENABLE_LLAMA_CPP=OFF is the default. With it ON, CMake fetches llama.cpp from https://github.com/ggml-org/llama.cpp.git at b4514 and links the llama target. The source is fetched at build time, not copied into this repository. Android can opt in with gradle assembleDebug -PseedxLlamaCpp=ON. CPU inference is enabled; GPU offload is intentionally disabled until device-specific validation exists.

## Model manifest and safety
Manifest includes ID, display name, absolute local path, format, optional byte size, estimated RAM, context length and a reserved SHA-256 field. Validation checks path, regular-file status, nonzero file length, GGUF magic and optional exact file size. SHA-256 is NOT yet calculated or verified. A valid GGUF header does not prove the model is trustworthy.

## Memory behavior
Available memory is read from /proc/meminfo; callers must still reserve memory for Android and other app processes. estimatedRamBytes must include weights, KV cache, runtime workspace and tokenizer overhead. A budget pass is not a guarantee against Android low-memory killing. Use one active model at a time and start with small quantized models and 2K–4K context on 6 GB devices.

## Inference behavior
Prompt is tokenized by llama.cpp. The runtime clears the KV cache before each request, evaluates the prompt, then samples one token at a time. Chunks are delivered to the callback; returning false cancels generation. Temperature zero selects greedy sampling; otherwise top-p, temperature and a seeded distribution sampler are used. Runtime access is serialized by its caller; concurrent load/unload/generate and reentrant calls from callbacks are unsupported.

## Security boundary
The model output is untrusted text. No model-generated shell commands or tool calls are executed. Any future tool proposal must be parsed as data and authorized by the independent Phase 2 Guardian before a separate executor can act.

## Not included
- SHA-256/signature verification and trusted model provenance.
- Model downloader, catalog, resume and update manager.
- Android SAF model picker and UI wiring.
- GPU/Vulkan/NNAPI/Hexagon acceleration.
- Durable conversation memory or multi-turn chat-template orchestration.
- Production inference benchmarks or real-device verification.

## Upstream
llama.cpp build documentation: https://github.com/ggml-org/llama.cpp/blob/master/docs/build.md
