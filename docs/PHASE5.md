# SEED-X Phase 5 — Android AI Studio

## Delivered in v0.5.0
- Native Android Java activity with five screens: Chat, Models, Providers, Guardian and Settings.
- Dark responsive UI, bottom navigation, message cards, provider status and model inventory.
- Android chat requests for OpenAI-compatible chat-completions, Anthropic Messages and Gemini generateContent.
- Explicit online fallback toggle, OFF by default.
- API key encryption using an AES-GCM key held by Android Keystore. Provider config contains an opaque credential reference, not the key.
- HTTPS-only endpoint validation, connection/read timeouts and bounded response reads.
- GGUF SAF import into app-private files/models storage; no broad storage permission.
- App-private chat history, capped at 100 messages.
- Guardian screen documents chat-only safety scope; it does not claim live C++ Guardian status.

## Android build
Windows PowerShell:
    gradle assembleDebug
    gradle assembleRelease

Optional llama.cpp build:
    gradle assembleDebug -PseedxLlamaCpp=ON

Install debug APK:
    adb install -r android/app/build/outputs/apk/debug/app-debug.apk

## Security and privacy
- API keys are encrypted with Android Keystore AES/GCM and a random IV. Keys are never written to provider configuration or chat history.
- Android backup is disabled.
- Online fallback is disabled by default and must be enabled by the operator.
- Only HTTPS endpoints are accepted by the Android UI.
- When enabled, prompts and recent conversation context are sent to the configured provider.
- Chat history is app-private; clear it from Settings.
- No AccessibilityService, shell executor, SMS, contacts or broad file permissions are requested.

## Explicit integration gaps
- Android provider HTTP client currently runs in Java separately from the C++ Phase 4 adapter. A unified JNI bridge remains future work.
- SSE streaming, request cancellation, retry/backoff and provider health probes are not included.
- GGUF import copies files to private storage, but the C++ LocalAiRuntime/llama.cpp backend is not connected to the Android chat screen through JNI. Import does not load or run a model.
- Guardian page describes the declared chat-only capability boundary; it does not query the live C++ Guardian object.
- No real credentials, live API calls, APK build or physical-device run are claimed by repository CI.

## Next engineering milestone
Phase 6 should connect Android to C++ LocalAiRuntime and Guardian through JNI, add streaming, model lifecycle status and cancellation, and run integration tests on Android devices.
