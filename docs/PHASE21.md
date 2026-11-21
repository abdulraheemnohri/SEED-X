# Phase 21 — AI Provider Expansion

## Implemented
- Existing provider catalog remains the single source for provider templates, endpoint kinds, default model IDs and access notes.
- Provider Hub retains encrypted API-key references using Android Keystore AES-GCM; provider profiles store only credential references.
- Added bounded transient retries for provider chat requests: configurable 0–3 retries, exponential delay capped at 4 seconds, and retry only for I/O failures and HTTP 429/5xx responses.
- Added local attempt/success/failure counters. The ledger stores no prompts, completions, keys, token contents or provider-side billing values.
- Retry count is configurable in Settings alongside timeout and response-size limits.
- Local inference remains preferred when a local model is loaded. Remote/local-server inference remains optional and requires explicit network fallback enablement and per-send confirmation.

## Provider protocol support
- OpenAI-compatible chat completions, Anthropic Messages and Gemini generateContent are implemented in the Android transport.
- Catalog entries using compatible endpoints are templates, not a guarantee that every model/provider supports identical request fields or tool calling.
- Ollama, LM Studio, llama.cpp server, LocalAI, vLLM and SGLang are remote/local-server connections; they are not bundled into the APK.
- Provider model names, free quotas, pricing, regions and terms can change. Verify them with the provider.

## Limits
- Usage counters are request-attempt counts, not token metering or invoices.
- Provider test requests are not included in the chat usage ledger.
- No automatic provider failover is performed; retries stay on the selected endpoint.
- Streaming transport and provider-specific structured tool calls are not claimed by this Android HTTP adapter.
