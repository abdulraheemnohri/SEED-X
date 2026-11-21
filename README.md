# SEED-X

**Self-Evolving Android AI Agent** — a local-first native Android AI agent foundation.

## Phase 1 — Native foundation
C++20 engine, Android NDK NativeActivity shell, OpenGL ES 3 rendering surface, lifecycle kernel, event bus and host-side tests.

## Phase 2 — Guardian security & governance
- Independent default-deny policy engine with scoped tool/operation/resource rules.
- Explicit operator approval tickets, bound to one request, short-lived and one-use.
- Emergency stop and bounded hash-linked audit trail.
- Host tests for policy, approval, emergency-stop and audit behavior.

## Phase 3 — Local AI Brain
- Backend-neutral C++20 inference interface and model lifecycle manager.
- Optional real llama.cpp backend pinned to upstream b4514; GGUF model loading and autoregressive token generation.
- Token streaming and cooperative cancellation callback.
- Sampling controls (temperature, top-p), context limits and prompt bounds.
- /proc/meminfo device-memory probe and RAM budget admission.
- GGUF magic, file type, size, path and context validation.
- Host-side deterministic fake-backend tests; llama.cpp integration is an opt-in build.
- Default build stays lightweight and has no network dependency.

## Phase 4 — Online AI Provider Layer
- Provider registry and normalized chat contract for OpenAI-compatible, Anthropic and Gemini APIs.
- DeepSeek, Qwen-compatible gateways and OpenRouter can use the OpenAI-compatible adapter where the service supports chat-completions.
- API keys are resolved by opaque credential references through CredentialVault; keys are never stored in provider configuration.
- HTTPS-by-default endpoint validation, bounded request settings and provider-reported usage accounting.
- Local-first routing; online fallback requires both explicit operator permission and network availability.
- Mock HTTP tests for provider payloads, auth headers, response parsing, usage, errors and routing.
- See docs/PHASE4.md for integration boundaries and limitations.

## Enable real local inference

The default build uses a no-backend stub so CI and the Android shell can compile without downloading a large inference dependency. To build with llama.cpp, allow CMake to fetch the pinned upstream source.

Windows PowerShell (host build):
```powershell
cmake -S . -B build-llama -DSEEDX_ENABLE_LLAMA_CPP=ON
cmake --build build-llama --config Release --parallel
ctest --test-dir build-llama -C Release --output-on-failure
```

Android ARM64 / emulator:
```powershell
gradle assembleDebug -PseedxLlamaCpp=ON
```

This fetches llama.cpp during CMake configuration and can take time. Android builds target arm64-v8a and x86_64. Keep model files in app-private storage and provide their absolute path to ModelManifest. The application must request model-storage access through Android's Storage Access Framework; do not hardcode shared-storage paths.

## Model sizing guidance

On a 6 GB RAM phone, begin with small quantized GGUF models and conservative context sizes (e.g. 2K–4K). The memory estimator is only an admission guard; it cannot guarantee that Android will not kill the process. Reserve RAM for Android, UI, KV cache, tokenizer and backend workspace. Do not load multiple models concurrently.

## Run host tests without llama.cpp

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Security boundary

Local and online model output is untrusted text. No model/provider output is executed as a shell command or tool action. Guardian policy changes and emergency-stop reset are never model-controlled. Future tool proposals must pass Phase 2 Guardian authorization and the one-use operator approval flow.

## Current limitations

Phase 4 supplies tested provider adapters and routing contracts, but does not yet include a production Android HTTP transport, Android Keystore CredentialVault implementation, app settings/chat UI, SSE streaming, retry/backoff or provider-native tool calling. No live provider request or APK network test is claimed. Phase 3 limitations also remain: SHA-256 model-file verification, signed model provenance, model downloader/catalog, Android model picker, GPU/Vulkan/NNAPI acceleration, persistent chat history and concurrent inference. The llama.cpp backend is real when explicitly enabled; default CI uses a deterministic test backend and stub factory.

## License
MIT. See LICENSE.
