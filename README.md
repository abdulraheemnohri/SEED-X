# SEED-X

**Self-Evolving Android AI Agent** — a local-first native Android AI agent foundation.

## Phase 1 — Native foundation
C++20 engine, Android NDK NativeActivity foundation, OpenGL ES 3 rendering surface, lifecycle kernel, event bus and host-side tests.

## Phase 2 — Guardian security & governance
- Independent default-deny policy engine with scoped tool/operation/resource rules.
- Explicit operator approval tickets, bound to one request, short-lived and one-use.
- Emergency stop and bounded hash-linked audit trail.
- Host tests for policy, approval, emergency-stop and audit behavior.

## Phase 3 — Local AI Brain
- Backend-neutral C++20 inference interface and model lifecycle manager.
- Optional real llama.cpp backend pinned to upstream b4514; GGUF loading and autoregressive token generation.
- Token callback streaming, cooperative cancellation, temperature/top-p sampling and context limits.
- Device memory probe, RAM admission and GGUF file validation.
- Host-side fake-backend tests; llama.cpp is opt-in.

## Phase 4 — Online AI Provider Layer
- C++ provider-neutral contract for OpenAI-compatible, Anthropic and Gemini.
- DeepSeek, Qwen-compatible gateways and OpenRouter can use OpenAI-compatible chat-completions where supported.
- Credential references, HTTPS-by-default validation, request bounds and provider-reported usage.
- Local-first routing; online fallback requires explicit permission and network availability.
- Mock HTTP adapter tests. See docs/PHASE4.md.

## Phase 5 — Android AI Studio
- Native Android Java activity and five screens: Chat, Models, Providers, Guardian and Settings.
- OpenAI-compatible, Anthropic and Gemini chat requests from Android.
- API keys encrypted with AES-GCM using Android Keystore; only opaque key references live in provider configuration.
- Online fallback is OFF by default and must be enabled by the operator.
- HTTPS-only endpoints, connection/read timeouts and 2 MB response cap.
- GGUF document picker copies files into app-private storage without broad storage permissions.
- Chat history stored locally and capped at 100 messages.
- See docs/PHASE5.md for build steps and integration gaps.

## Build on Windows PowerShell

Host tests:
```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Android debug APK:
```powershell
gradle assembleDebug
```

Optional llama.cpp backend:
```powershell
gradle assembleDebug -PseedxLlamaCpp=ON
```

Install:
```powershell
adb install -r android/app/build/outputs/apk/debug/app-debug.apk
```

On a 6 GB RAM phone, begin with small quantized GGUF models and 2K–4K context. The Phase 5 model picker imports files but does not yet connect them to Android inference.

## Security boundary
Local and online model output is untrusted text. No model/provider output is executed as a shell command or tool action. Guardian policy changes and emergency-stop reset are never model-controlled. The Android chat UI is deliberately chat-only and does not expose device-control actions.

## Current limitations
Phase 5's Android provider client is not yet wired to the C++ Phase 4 provider adapter. SSE streaming, request cancellation, retries, provider health checks and persistent usage ledger are pending. The Android GGUF picker does not yet connect to C++ LocalAiRuntime through JNI; the Guardian page does not query the live C++ Guardian object. No live provider API, APK build or physical-device test is claimed by this repository update. Phase 3 still needs model SHA-256 verification, signed model provenance, downloader/catalog and GPU acceleration.

## License
MIT. See LICENSE.
