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

## Phase 4 — Online AI Provider Layer
- Provider-neutral C++ contract for OpenAI-compatible, Anthropic and Gemini.
- DeepSeek, Qwen-compatible gateways and OpenRouter can use OpenAI-compatible chat-completions where supported.
- Credential references, HTTPS-by-default validation, request bounds and provider-reported usage.
- Local-first routing policy and explicit online fallback permission.
- Mock HTTP adapter tests. See docs/PHASE4.md.

## Phase 5 — Android AI Studio
- Native Android Java UI: Chat, Models, Providers, Guardian and Settings.
- OpenAI-compatible, Anthropic and Gemini chat requests.
- Android Keystore AES-GCM credential encryption; online fallback OFF by default.
- GGUF document picker and app-private model storage.
- See docs/PHASE5.md.

## Phase 6 — JNI and live native integration
- Java UI loads the native C++ shared library and initializes AgentKernel.
- JNI connects GGUF model loading, local generation, token callbacks and cooperative cancellation to LocalAiRuntime.
- Local model is selected before cloud providers.
- Guardian screen reads live emergency-stop and audit-chain status; explicit operator confirmation controls stop/reset.
- See docs/PHASE6.md.

## Build on Windows PowerShell

Host tests:
    cmake -S . -B build
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

Android debug APK with JNI bridge and stub backend:
    gradle assembleDebug

Android debug APK with actual llama.cpp GGUF inference:
    gradle assembleDebug -PseedxLlamaCpp=ON

Install:
    adb install -r android/app/build/outputs/apk/debug/app-debug.apk

For a 6 GB RAM phone, start with small quantized GGUF models and a 2K context. The app estimates memory conservatively, but real device testing is still required.

## Security boundary
System 1 (Guardian) owns policy and authorization. System 2 model output is untrusted text. No model/provider output is executed as a shell command or tool action. Emergency-stop reset is operator-confirmed; model output cannot modify policies or reset Guardian.

## Current limitations
- Android local inference requires -PseedxLlamaCpp=ON; default CI compiles the JNI bridge with a stub backend.
- The online Java provider client and C++ Phase 4 provider adapter are not yet unified.
- Full policy editor, approval-ticket UX and audit-log browser are pending.
- No GPU/NPU acceleration or physical-device benchmark is claimed.
- SHA-256 model verification, signed model provenance and model catalog/downloader remain pending.

## License
MIT. See LICENSE.
