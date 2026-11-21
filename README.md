# SEED-X

**Self-Evolving Android AI Agent** — a local-first native Android AI workspace and Guardian-governed tool platform.

![SEED-X logo](assets/brand/seedx-logo.svg)

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
- Native Android Java UI: Home, Chat, Voice, Memory, Models, Runtimes, Providers, Skills, Workflows, Tools, Guardian, Diagnostics and Settings.
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

## Phase 7 — Guardian-Gated Tool Execution
- C++ typed tool registry with no shell or arbitrary command handler.
- Android Tools tab for device/runtime status and private-workspace listing/reading.
- New note creation is limited to the app-private workspace and requires a one-use, 60-second, request-bound Guardian approval.
- Guardian permission dashboard and recent audit-history viewer.
- Atomic authorization-and-execution boundary: emergency stop prevents new handlers and waits for an in-flight bounded handler to finish.
- Workspace path traversal protection, 1 MiB read cap, 64 KiB write cap, exclusive no-overwrite note creation.
- Host tests cover tool registration, approvals, replay rejection and emergency-stop blocking.
- See docs/PHASE7.md.

## Phase 8 — Universal AI Control Center (0.8.0)
- Dashboard, Chat, Voice, Memory, Models, Runtimes, Providers, Skills, Workflows, Tools, Guardian, Diagnostics and Settings screens.
- Multi-profile provider hub with presets for Gemini, Groq, OpenRouter, Cloudflare Workers AI, NVIDIA NIM, Z.AI, SambaNova, Cohere, Cerebras, OpenAI, Anthropic, DeepSeek, Mistral, Qwen, Together, Fireworks, Perplexity, xAI, Hugging Face Router, Ollama, LM Studio, llama.cpp server, LocalAI, vLLM and SGLang.
- Free-tier labels are qualified and limited; the app bundles no API keys and never promises unlimited free inference. Cloud prompts require explicit per-send confirmation.
- Local HTTP is opt-in and restricted in app code to loopback/private IPv4/.local endpoints. HTTPS remains required for public providers. Keyless local OpenAI-compatible servers are supported.
- Runtime catalogue covers llama.cpp, LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI, MNN-LLM, ncnn, Ollama, LM Studio, LocalAI, vLLM, SGLang, OpenVINO and TensorRT-LLM. llama.cpp is the embedded GGUF path; LiteRT-LM and ExecuTorch have Phase 29 adapters that still require build/device certification. Android SpeechRecognizer/TTS and the optional Puter.js WebView are integrated. Other audio, vision and multimodal runtimes remain catalogue entries until their adapters are implemented and tested.
- Twenty built-in prompt skills with search and enable/disable, six manual workflows, user-managed opt-in local memory, speech-to-text handoff and Android text-to-speech.
- Configurable system prompt, output tokens, temperature, top-p, context, provider timeout and response-size bounds.
- Device-aware model guide for constrained phones, imported GGUF model management, model deletion, runtime diagnostics and copyable privacy-safe diagnostics.
- Android launcher logo: assets/brand/seedx-logo.svg and android/app/src/main/res/drawable/seedx_logo.xml.
- Provider/runtime/skill/model catalogs are in config/ and bundled Android assets. See docs/PHASE8.md.

## Build on Windows PowerShell

Host tests:
    cmake -S . -B build
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

Complete Android debug APK with embedded llama.cpp GGUF inference (recommended):
    gradle clean assembleDebug -PseedxLlamaCpp=ON -PseedxRuntimeSdks=litert,executorch,onnxruntime

Lightweight debug APK with JNI stub backend (no embedded inference):
    gradle clean assembleDebug -PseedxLlamaCpp=OFF

Install the APK produced by the selected build:
    adb install -r android/app/build/outputs/apk/debug/app-debug.apk

GitHub Actions publishes the llama.cpp-enabled build as `seedx-debug-apk` and the smaller stub build as `seedx-debug-apk-stub`. The complete APK includes the app UI, AI/voice runtime catalogs, provider/backend management, device status, Android system ASR/TTS, optional Puter.js WebView, the compiled CPU llama.cpp adapter, and optional LiteRT-LM/ExecuTorch/ONNX SDK packages when selected. It does not bundle provider API keys or large model weights; import a licensed GGUF model separately. Other catalog runtimes still require their own SDK-backed adapters.

For a 6 GB RAM phone, start with small quantized GGUF models and a 2K context. The app estimates memory conservatively, but real device testing is still required.

## Security boundary
System 1 (Guardian) owns policy and authorization. System 2 model output is untrusted text. No model/provider output is executed as a shell command or tool action. Emergency-stop reset is operator-confirmed; model output cannot modify policies or reset Guardian.

## Current limitations
- The default APK uses the JNI stub backend. CI also attempts a second Android build with embedded llama.cpp and uploads it as `seedx-debug-apk-llama` if the NDK build succeeds. Locally enable it with `-PseedxLlamaCpp=ON`.
- The Android Java provider transport and C++ Phase 4 provider adapter remain separate implementations.
- Policy dashboard is read-only; a visual policy editor and third-party dynamic plugin loading are pending.
- Model-initiated autonomous tool planning is not enabled; operators launch tools from the Tool Center. No GPU/NPU acceleration or physical-device benchmark is claimed.
- SHA-256 model verification, signed model provenance and an in-app model downloader remain pending.

## Project audit
A repository-wide architecture, implementation, security and release-readiness review is available in [docs/PROJECT_AUDIT_2026-11-21.md](docs/PROJECT_AUDIT_2026-11-21.md). It distinguishes implemented features from catalog-only runtimes and documents remaining release, security and device-testing work.

## License
MIT. See LICENSE.


## Phase 9 — Guardian-reviewed tool proposals

Phase 9 adds strict JSON proposal review in the Android Tool Center, mandatory operator confirmation, one-use Guardian approval for workspace writes, a bounded sandboxed workspace search tool, and Provider Hub presets loaded directly from the provider catalog. See [docs/PHASE9.md](docs/PHASE9.md) for implemented scope and explicit runtime/provider limitations.


## Phase 10 — Runtime Registry

Added a native runtime registry and live JNI availability reporting. Runtime Manager now marks a backend runnable only when a native factory is registered and compiled. LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI and MNN-LLM remain explicitly unavailable until their SDK-backed adapters are implemented. See [docs/PHASE10.md](docs/PHASE10.md).


## Phase 11 — Model format contracts

Added explicit model package format identifiers and hardened runtime factory validation. A runtime cannot be instantiated unless its factory returns a backend compatible with its declared formats. GGUF remains the only currently validated local model format; ONNX, LiteRT, MLC, ExecuTorch, MNN and ncnn are recognized but not yet executable. See [docs/PHASE11.md](docs/PHASE11.md).


## Phase 12 — Android OS integration

Added a user-started Android foreground-session service, notification permission flow, Android app-permission settings shortcuts, and battery-optimization settings access. The service is a lifecycle foundation only: it does not execute tools or access privileged device APIs. It remains subject to Android background-start and foreground-service limits. See [docs/PHASE12.md](docs/PHASE12.md).


## Phase 14 — Crash and bug reporting

Added local fatal-crash capture, explicit non-fatal error recording, diagnostic preview/copy/delete controls, and a privacy-reviewed GitHub issue draft handoff. Reports are never uploaded automatically and exclude chat history, model files, provider credentials and API keys. See [docs/PHASE14.md](docs/PHASE14.md).


## Phase 15 — Diagnostics Center

Expanded Diagnostics with crash/error history, recent screen-navigation breadcrumbs, Guardian audit verification and reviewed GitHub issue drafts. Added a local breadcrumb privacy setting and bounded retention. No automatic telemetry or self-modifying repairs. See [docs/PHASE15.md](docs/PHASE15.md).


## Phase 16 — Error Intelligence & CI Evidence

Added heuristic error categories, normalized duplicate-crash fingerprints, a duplicate grouping view, complete local diagnostic clearing, and GitHub Actions core-job summaries. Native signal crash dumps and ANR capture remain explicitly unsupported. See [docs/PHASE16.md](docs/PHASE16.md).


## Phase 21 — AI Provider Expansion
Added bounded retry handling for transient provider/network failures, configurable retry limits, and privacy-safe local attempt/success/failure counters. Provider secrets remain in Android Keystore and remote fallback remains opt-in. See [docs/PHASE21.md](docs/PHASE21.md).

## Phase 22 — Android Operations UI
Added Permission Center, foreground-session Task Manager, curated GGUF Model Download Manager, high-contrast text and adjustable text sizing. The download flow uses app-specific storage and validates GGUF magic bytes before private-library import. Scheduled WorkManager queues, full theme switching and visual workflow editing are still pending. See [docs/PHASE22.md](docs/PHASE22.md).

## Phase 23 — Release Engineering
CI now validates JSON catalogs and the declared Android permission boundary, builds debug APK plus unsigned release APK/AAB artifacts, and uploads release artifacts. Added a security policy. Release signing and physical-device validation remain maintainer actions. See [docs/PHASE23.md](docs/PHASE23.md) and [SECURITY.md](SECURITY.md).


## Support development
SEED-X is free and open source. Optional donations help fund testing, model/runtime integration and maintenance. No subscription is required by this project.

- [GitHub Sponsors](https://github.com/sponsors/abdulraheemnohri)


## Phase 24–25 — Mobile UI and navigation
The Android app now uses a four-tab bottom navigation bar, grouped hamburger menu, live screen search, favourite screens, a Home quick-launch grid, configurable startup destination, and menu actions for Guardian emergency stop and generation cancellation. CI validates the navigation entry points and screen registry. See [Phase 24](docs/PHASE24.md) and [Phase 25](docs/PHASE25.md).


### Mobile navigation and settings
The Android app includes a compact four-tab bottom bar, grouped mobile drawer, screen search, favourites, startup-screen selection and quick-launch dashboard. Settings can be exported/imported as a validated JSON backup through Android's document picker. Provider secrets, provider profiles, chat history and model files are intentionally excluded.


### Themes and command palette
SEED-X supports dark, light and Android-system themes, a searchable command palette with fixed guarded actions, and independently configurable Home dashboard widgets. Theme and dashboard preferences are supported by the validated JSON settings backup.


### Task reminders and workflow plans
SEED-X can schedule periodic local reminders through Android WorkManager, keep a bounded local task event history, and create structured linear workflow plans. Reminder workers only show notifications; they do not invoke providers or execute tools. Provider connection diagnostics and native runtime status probes are available in the app.

## Additional mobile workspace screens
The app now includes Prompt Library (built-in and custom local templates), conversation search and Markdown export, Storage Manager, read-only Network Diagnostics, and About/Support. Prompt selection only fills the composer; no prompt is sent automatically. Storage tools operate only on SEED-X app-owned data. Settings backup includes custom prompts and dashboard visibility preferences, but never exports provider credentials, chat history or model files.

## Adaptive UI expansion
- Replaced the fixed phone-only shell with width-aware navigation: a five-item bottom bar on compact windows and a persistent grouped side rail on large windows.
- Added automatic layout selection plus user overrides for automatic, bottom-bar and side-rail navigation.
- Added comfortable/compact content density, responsive dashboard quick-launch columns (2/3/4), clearer active navigation states and consistent touch targets.
- Added resizable-activity and configuration handling so rotation, foldable/window-size changes rebuild the adaptive shell.
- Improved light-theme system-bar icon contrast, card spacing, control contrast and screen-reader content descriptions for navigation actions.
- Navigation mode and density are included in the validated settings backup.
- Device screenshots, TalkBack traversal and multi-device manual verification remain outstanding; the current implementation is native Android Views, not a Compose redesign.


## Phase 26 — Device Status & Backend Management
- Added a Device Status screen for Android identity/API level, ABI and CPU-core information, battery percentage/charging source/health/temperature/voltage, battery saver, battery-optimization exemption, thermal status where supported, system RAM pressure, internal storage, SEED-X model/download storage usage, and reported hardware features.
- Expanded Runtime Manager with catalog-versus-runnable counts, category/acceleration metadata, copyable details, refresh and diagnostics actions. Added a Backend Manager that separates embedded runtime state from remote/local provider profiles and exposes explicit online-fallback and private-LAN HTTP controls.
- Added navigation and Home quick-launch entries for Device Status and Backend Manager.
- Permission Center now enumerates the manifest-declared Android permissions and shows their current grant/normal state. No additional permissions were added; device information is read through public status APIs, with no radio toggles or privileged controls.
- Provider templates remain compatibility presets. The current catalog includes 24 known provider profiles, with custom OpenAI-compatible endpoints supported; account availability, model aliases, pricing, regional support and free quotas must be verified with each provider.
- Runtime catalog entries remain distinct from runnable adapters. llama.cpp is the embedded engine available in the optional build; other listed runtimes must not be treated as installed or executable until their adapters are integrated and verified.
- Device-specific hardware readings, thermal reporting and power behavior require validation on physical Android devices. See [docs/PHASE26.md](docs/PHASE26.md).


## Phase 27 — Complete Debug APK
The recommended GitHub Actions debug artifact is now the llama.cpp-enabled `seedx-debug-apk`, with a smaller `seedx-debug-apk-stub` fallback. See [docs/PHASE27.md](docs/PHASE27.md) for exactly what is included and which runtime adapters remain pending.


## Phase 28 — Modular Android Runtime SDKs

The complete debug build now opts into the LiteRT-LM Android SDK, ExecuTorch Android AAR, and ONNX Runtime Mobile AAR. Individual SDKs can be selected with the `seedxRuntimeSdks` Gradle property. The Runtime Manager probes SDK classes and explicitly distinguishes packaged SDKs from runnable SEED-X adapters.

Example:
```powershell
gradle clean assembleDebug -PseedxLlamaCpp=ON -PseedxRuntimeSdks=litert,executorch,onnxruntime
```

Important: SDK packaging is not the same as an integrated generation adapter. LiteRT-LM and ExecuTorch still need tested SEED-X model lifecycle/generation bridges. ONNX Runtime Mobile is not ONNX Runtime GenAI. MLC LLM requires its generated `mlc4j` project and per-model compiled libraries. See [docs/PHASE28.md](docs/PHASE28.md).


## Phase 29 — Real LiteRT-LM and ExecuTorch inference adapters

- Adds a common Java inference contract and an actual LiteRT-LM Kotlin conversation adapter for external `.litertlm` model packages.
- Adds an ExecuTorch Android LLM adapter for `.pte` model files plus their matching tokenizer assets. ExecuTorch Android LLM API is experimental and invoked through a compatibility reflection bridge.
- Adds local model import through Android document picker, app-private model storage, automatic format-based runtime selection, one-active-runtime policy, model memory admission checks, token streaming, cancellation and explicit unload.
- Chat routes to a loaded Phase 29 adapter after Guardian status is verified; existing GGUF inference remains on the llama.cpp JNI path.
- LiteRT-LM is pinned to 0.16.1 to keep its Kotlin metadata compatible with the Kotlin 2.2 toolchain; the app includes Kotlin Android plugin and coroutines.
- A matching tokenizer is mandatory for ExecuTorch `.pte` models. Model weights are not bundled into the APK. LiteRT-LM defaults to CPU; GPU/NPU selection remains a separate device-capability validation step.
- CI and physical-device inference verification are required before treating either adapter as release-certified.
See [docs/PHASE29.md](docs/PHASE29.md).


## Phase 30 — Voice, universal AI capabilities and Puter.js

- Voice Studio uses Android SpeechRecognizer with an explicit on-device-only mode when supported and an explicitly selected system recognizer mode. There is no silent network fallback from on-device mode.
- Microphone permission is declared but requested only when the user starts ASR. Recognition is stopped when leaving the Voice screen or pausing the activity. No background microphone capture or wake-word service is enabled.
- TTS uses the installed Android TextToSpeech engine with language, speed and pitch controls.
- Adds voice runtime inventory for Android ASR/TTS, Whisper.cpp, sherpa-onnx, Vosk, Moonshine, Silero VAD, Piper, Kokoro and ML Kit GenAI Speech Recognition. Entries marked catalog-only or alpha are not claimed as integrated.
- Adds an AI capability catalogue covering text/reasoning, vision-language, ASR, TTS, speech translation, VAD/wake word, diarization, audio understanding, embeddings, reranking, OCR, image/video generation and Guardian-gated agents.
- Adds an optional Puter.js WebView workspace for chat, model listing, text-to-speech, speech-to-text from a public HTTPS audio URL, image understanding and image generation. It uses the official Puter.js v2 script and does not expose a native JavaScript bridge or store provider API keys.
- Puter is an online user-pays integration, not an offline runtime. The workspace asks for confirmation before online requests. Verify Puter account terms, current model pricing and privacy before use.
- Voice preferences are included in the allowlisted settings backup. Provider credentials, conversation history and model files remain excluded.
- Adds browser AI runtime inventory for WebLLM, Transformers.js, ONNX Runtime Web, MediaPipe Tasks Web and TensorFlow.js.

Runtime status honesty: Android SpeechRecognizer and Android TTS are platform integrations. LiteRT-LM and ExecuTorch adapters from Phase 29 are implemented but still require build/device certification. Whisper.cpp, sherpa-onnx, Vosk, Piper, Kokoro and other catalogue-only entries need concrete adapters and model packaging before they can run in SEED-X.

Official references:
- [Puter.js AI docs](https://docs.puter.com/AI/)
- [Android SpeechRecognizer API](https://developer.android.com/reference/android/speech/SpeechRecognizer)
- [ML Kit GenAI Speech Recognition](https://developers.google.com/ml-kit/genai/speech-recognition/android)
- [ML Kit GenAI Text-to-Speech](https://developers.google.com/ml-kit/genai/text-to-speech/android)


## Phase 31 — Universal Chat and SEED Growth

See [docs/PHASE31.md](docs/PHASE31.md) for the full feature list, attachment limits, connector status and remaining multimodal integration work. The Chat screen now supports local/provider model selection, multiple local conversations, file attachments, push-to-talk voice chat, optional spoken replies, and shortcuts to skills, Guardian tools, workflows and integrations. The new Growth screen records human-reviewed improvement proposals without allowing the model to modify trusted System 1 code.
