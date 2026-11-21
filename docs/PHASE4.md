# SEED-X Phase 4 — Online Provider Layer

## Delivered
- Provider-neutral chat contract and registry for OpenAI-compatible APIs, Anthropic Messages, and Google Gemini generateContent.
- DeepSeek, Qwen-compatible gateways, OpenRouter and other OpenAI-compatible services can be configured with a compatible base URL and model identifier.
- Credential references are resolved through CredentialVault; provider configs contain references, never API-key values.
- HTTPS is mandatory by default. Cleartext HTTP requires explicit opt-in and is restricted to local/private endpoint prefixes.
- Timeout, response-byte, output-token and temperature bounds; errors do not echo credentials or request headers.
- Local-first routing: a ready local model always wins. Online use requires explicit operator enablement and network availability.
- Provider-reported token usage is captured when present; no fabricated usage numbers.
- Mock-transport tests cover routing, payloads, auth headers, missing credentials, HTTP errors and response extraction.

## Configuration and secrets
Each provider has an id, display name, kind, HTTPS base URL, model id, credential reference, enabled flag, timeout and response-size cap. CredentialVault is a platform boundary. Android production integration must implement it with Android Keystore-backed encryption and must never persist secrets in plain preferences, logs, source files or exported backups. Host tests use an in-memory fake vault only.

## Network transport boundary
HttpTransport is injected. The C++ engine builds provider-specific JSON requests and validates responses, while the Android host must supply a production transport with certificate validation, redirect restrictions, cancellation, bounded reads and secure connection handling. This phase does not yet ship that Android transport bridge or a CredentialVault implementation; provider adapters are mock-tested but not yet wired to live API calls from the APK.

## Provider notes
- OpenAI-compatible: /chat/completions and bearer authentication. Compatible DeepSeek/Qwen gateways can use this adapter when they implement chat completions.
- Anthropic: /v1/messages, x-api-key and anthropic-version headers.
- Gemini: /v1beta/models/{model}:generateContent and x-goog-api-key header.
- Common response text shapes and provider-reported usage are extracted. Tool/function calls are not executed.
- SSE streaming, native tool calls, image/audio payloads, retry/backoff, live connection UI and durable usage ledger are deferred.

## Security boundary
Provider output is untrusted. No provider can modify Guardian policies, clear emergency stop or execute commands. Future action proposals must be data and pass the independent Phase 2 Guardian authorization and one-use operator approval flow.

## Build and test
PowerShell:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure
