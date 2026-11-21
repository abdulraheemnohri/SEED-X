# SEED-X Phase 3 — Local AI Runtime Foundation

## Delivered
- Backend-neutral C++20 inference interface.
- GGUF model manifest validation and local regular-file checks.
- Device-memory budget checks with a configurable reserve.
- Candidate filtering ordered by estimated RAM use.
- Explicit backend load/unload lifecycle.
- Streaming callback contract; callback can cancel generation.
- Prompt and generation-parameter bounds.
- Runtime state tracking and exception containment.
- Host tests using a deterministic test backend.

## Runtime boundary
The repository now defines the integration contract; it does not yet ship a production inference backend. The test backend exists only in tests and must never be represented as actual AI inference. A compatible llama.cpp backend is the next implementation task. Android builds must package or build that backend separately for each ABI.

## Model manifest
A model manifest contains a stable ID, display name, local file path, GGUF format, file size, estimated RAM use and context-token limit. The current validator checks file existence, regular-file status, nonzero size and optional exact file-size match.

The sha256 field is reserved for future integrity verification. It is not verified in this phase. Do not load untrusted model files in a production deployment until cryptographic hash verification and provenance checks are implemented.

## Memory budget
The caller supplies an available-memory estimate and a reserve that must remain unused. A model is considered eligible only when its estimated RAM requirement fits below available memory minus reserve. This is a conservative admission check, not a guarantee against Android process memory pressure. Real model sizing must account for weights, KV cache, context length, backend workspace, tokenizer and concurrent app memory.

## Streaming
Backends emit text chunks through a callback. Returning false requests generation cancellation. This API does not define tool calling or execute model-suggested commands. All future tool proposals must pass through the independent Phase 2 Guardian.

## Known limitations
- No llama.cpp or other production inference backend is bundled yet.
- No model download, catalog network service, checksum verification or signature validation.
- No Android-specific memory probe, NNAPI/GPU acceleration or background model loading.
- Runtime calls are currently intended to be serialized by the caller; concurrent load/unload/generate calls are not supported.
- No APK or physical-device inference test is claimed.

## Next
Integrate llama.cpp as an optional Android backend, expose model download and integrity verification, add device-memory telemetry, and connect a proper chat UI without giving System 2 authority over Guardian policy.
