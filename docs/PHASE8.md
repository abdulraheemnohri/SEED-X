# SEED-X Phase 8 — Universal AI Control Center

Version 0.8.0

## Scope
Phase 8 expands the existing C++20 / JNI / Android project with a provider/runtime/skills catalog, multiple provider profiles, a local-first configuration surface, workflow prompt skills, diagnostics, and a branded Android identity. Catalog entries are metadata and configuration templates; they do not imply every backend is integrated.

## Provider support matrix
The existing live chat transports support:
- Gemini native generateContent.
- Anthropic Messages API.
- OpenAI-compatible chat completions.

Provider presets using the OpenAI-compatible protocol include Groq, OpenRouter, Cloudflare Workers AI, NVIDIA NIM, Z.AI, SambaNova, Cohere compatibility, OpenAI, DeepSeek, Mistral, Qwen DashScope, Ollama, LM Studio, llama.cpp server, LocalAI, vLLM and SGLang. Each provider may require its own endpoint, model identifier, account, API key, headers or model-specific settings. Cloudflare requires replacing the account-ID placeholder. Always verify provider documentation before use.

Free-access labels are indicative, not guarantees:
- Gemini offers a free tier for selected models; free-tier data may be used to improve Google products.
- Groq has account/model-specific rate limits.
- OpenRouter offers free model variants with daily and per-minute caps.
- Cloudflare Workers AI offers 10,000 free Neurons/day; some models require billing.
- NVIDIA NIM offers developer prototyping access with model-specific limits.
- Z.AI, SambaNova and Cohere entries are marked verify-current-terms/trial rather than guaranteed unlimited free.
- OpenAI, Anthropic, DeepSeek, Mistral and Qwen entries are BYOK/plan-dependent; check current pricing and terms.
- No API keys are bundled. The operator obtains and stores their own credentials.

## Local runtimes
- llama.cpp is the only embedded local LLM backend integrated into the current C++ runtime, optionally built with -PseedxLlamaCpp=ON.
- LiteRT-LM, MLC LLM, ExecuTorch and ONNX Runtime GenAI are catalogued for future adapters. They are not falsely represented as active.
- Ollama, LM Studio, LocalAI, llama.cpp server, vLLM and SGLang are external local/LAN servers reachable through compatible API profiles; they are not bundled into the APK.

## Security and network policy
- Cloud fallback is disabled by default and local model routing remains first.
- API credentials remain encrypted by Android Keystore AES-GCM; only credential references are stored in profile JSON.
- HTTPS is mandatory for public providers.
- Cleartext HTTP is accepted only for explicitly enabled local/LAN endpoints whose host is loopback, RFC1918 private IPv4 or .local. This is an opt-in convenience; use TLS for untrusted networks.
- Redirects are disabled, response reads are bounded, and API errors must not print credentials.
- Provider terms and privacy differ. Do not send sensitive/private data to cloud free tiers without reviewing retention/training terms.

## Skills and workflows
Twenty built-in skills are searchable and can be individually enabled/disabled. They are prompt templates, not executable plugins. A manually managed memory screen stores up to 100 user-approved text entries locally; memory is disabled in prompts until the operator opts in. Voice uses Android's speech-recognition activity and TTS engine; there is no background microphone service. Six workflow templates are manually started and do not create hidden background automation. Tools remain a separate native allowlist.

## Runtime configuration
The app exposes max output tokens, temperature, top-p, context size, provider timeout, response-size cap, editable system prompt, online fallback, local HTTP, chat retention and memory opt-in. Provider profiles are stored separately; secrets remain in the Keystore vault. The Home dashboard, Voice, Memory, Skills, Workflows, Runtime Manager and Diagnostics screens are included. Values are bounded before use. The defaults are local-first, 512 output tokens, temperature 0.7, top-p 0.9, 2048 context, 768 MiB system reserve and online fallback disabled.

## Brand
- Vector source: assets/brand/seedx-logo.svg
- Android launcher drawable: android/app/src/main/res/drawable/seedx_logo.xml

## Build
    gradle assembleDebug
    gradle assembleDebug -PseedxLlamaCpp=ON
    adb install -r android/app/build/outputs/apk/debug/app-debug.apk

## Known limitations
- LiteRT-LM, MLC LLM, ExecuTorch and ONNX Runtime GenAI still need native runtime adapters, dependency pinning, model-format support, device tests and CI.
- Generic OpenAI-compatible endpoints are not guaranteed to support identical models, tool calling, multimodal input, streaming or structured outputs.
- The Android app currently sends non-streaming cloud chat completions; native local llama.cpp token streaming remains available.
- Provider model IDs and free tiers change. Catalog metadata must be reviewed and updated over time.
- Cloud provider requests are not automatically routed to a "free" model; the operator selects the provider and model.
- Skills are prompt templates, not independently sandboxed code packages.
- Voice recognition and speech synthesis depend on installed Android system services and language packs.\n- No physical device benchmark is claimed.
