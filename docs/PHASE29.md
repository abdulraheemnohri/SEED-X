# Phase 29 — LiteRT-LM and ExecuTorch local inference

## Delivered in this phase

### Shared adapter contract
`InferenceAdapter` defines model loading, generation callbacks, cancellation, unload and loaded-state reporting. The Android chat UI can route requests through a loaded adapter without depending on the concrete engine.

### LiteRT-LM
- Concrete Kotlin adapter using Google's LiteRT-LM Engine and Conversation APIs.
- Loads external `.litertlm` files from app-private storage.
- Initializes the engine on a dedicated single-thread executor, not the UI thread.
- Streams Flow message chunks to the common token listener.
- Uses `cancelProcess()` for cancellation and closes Conversation before Engine.
- Uses CPU backend by default. GPU/NPU are deliberately not selected automatically without device-specific validation.
- SDK version pinned to 0.16.1 for compatibility with the Kotlin 2.2.20 Android plugin.

### ExecuTorch
- Concrete adapter for ExecuTorch's experimental Android LLM `LlmModule` API.
- Loads `.pte` program files and a separately selected matching tokenizer file.
- Calls explicit `load()`, `generate()`, token callbacks and `stop()`.
- Uses reflection so a lightweight build without the optional ExecuTorch AAR can still compile; the adapter reports an unavailable SDK when the AAR is not packaged.
- The upstream Android LLM API is experimental and may change between releases. Release certification requires the exact SDK artifact and model/tokenizer pair to pass device tests.

### Android integration
- Models screen offers import for `.litertlm` and `.pte` packages.
- Model files and tokenizer files are copied to app-private storage; no weights are embedded in the APK.
- `.litertlm` automatically selects LiteRT-LM. `.pte` automatically selects ExecuTorch and prompts for its tokenizer.
- Chat sends to a loaded Phase 29 adapter after Guardian/native initialization and emergency-stop checks. Existing GGUF continues to use the llama.cpp JNI path.
- Chat exposes a stop-generation action and the Runtime screen reports SDK/adapter status.
- One active third-party runtime is retained at a time. A conservative admission check compares estimated model memory against Android's currently available system RAM.

## Model compatibility

| Runtime | Required file(s) | Format |
|---|---|---|
| LiteRT-LM | One compatible model package | `.litertlm` |
| ExecuTorch LLM | Exported program plus matching tokenizer | `.pte` + tokenizer |
| Existing llama.cpp | Existing model loader | `.gguf` |

A model's extension alone does not prove compatibility. LiteRT-LM packages must be built for LiteRT-LM; ExecuTorch files must be exported for the matching LLM runner and accompanied by the tokenizer and any model-specific assets required by that export.

## Memory and lifecycle

The manager unloads the previous third-party model before admitting another model. It estimates a conservative working set as model-file size × 1.2 + 384 MiB and compares it with current Android available RAM. This is an admission heuristic, not an exact peak-memory prediction; native allocations, KV cache, context length, delegates and thermal conditions vary by model/device.

Only one third-party adapter is active. Generation is dispatched away from the UI thread. Stop requests are forwarded to the runtime. Conversation/engine release is ordered to avoid closing LiteRT-LM while a stream is still active. ExecuTorch's Android LLM API does not expose a stable cross-version close contract in the integration guide; the adapter attempts supported release methods reflectively and drops the native module reference otherwise.

## Build

The project uses Kotlin Android plugin 2.2.20 and LiteRT-LM 0.16.1. The full debug workflow also opts into ExecuTorch and ONNX Runtime Mobile:

```powershell
gradle clean assembleDebug -PseedxLlamaCpp=ON -PseedxRuntimeSdks=litert,executorch,onnxruntime
```

LiteRT-LM is always included because its concrete adapter is compiled into the app. ExecuTorch and ONNX Runtime Mobile remain controlled by `seedxRuntimeSdks`.

## Verification status and remaining work

- Repository-level static guards now check the adapter classes, model-format selection, model import, chat routing and cancellation controls.
- A successful Gradle build is still required to validate Kotlin/Java API compatibility and dependency resolution.
- Device testing is still required for real model load, first-token latency, token throughput, cancellation latency, repeated load/unload, memory pressure, thermal behavior and app background/foreground transitions.
- No GPU/NPU acceleration or performance claim is made.
- ONNX Runtime Mobile, MLC LLM, MNN-LLM and ncnn remain separate future adapter work; they are not marked as operational by this phase.

## Upstream API references

- LiteRT-LM Android guide: https://developers.google.com/edge/litert-lm/android
- LiteRT-LM Android Kotlin integration guidance: https://developer.android.com/agents/skills/device-ai/litert/litert-lm/skill
- ExecuTorch Android LLM guide: https://docs.pytorch.org/executorch/stable/llm/run-on-android.html
