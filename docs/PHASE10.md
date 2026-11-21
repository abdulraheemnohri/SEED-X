# SEED-X Phase 10 — Runtime Registry and Live Availability

## Delivered

### Native runtime registry
- Added `RuntimeRegistry` as the native source of truth for runtime availability.
- Each registration has a stable ID, display name, category, supported model formats, availability state, embedded/remote classification, explanation, and optional backend factory.
- Duplicate IDs and invalid registrations are rejected.
- A backend marked Available must provide a factory; a factory cannot be attached to an unavailable runtime.
- `create(id)` returns a backend only when the runtime is explicitly available.
- Unknown, adapter-missing, optional-build-disabled and remote-service entries never create an embedded backend.

### Built-in registry entries
- llama.cpp: Available only when compiled into the build; otherwise OptionalBuildDisabled.
- LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI and MNN-LLM: AdapterMissing.
- Ollama and LM Studio: RemoteService, not embedded runtimes.

### Android Runtime Manager
- Added JNI runtime catalog endpoint backed by the C++ registry.
- Runtime Manager combines catalog descriptions with live native availability.
- A runtime is labelled AVAILABLE NOW only when the native registry reports `canRun=true`.
- Catalog-only integrations are explicitly labelled NOT RUNNABLE.

### Tests
- Added runtime registry host tests for invalid and duplicate registrations, factory requirements, runtime creation, unavailable runtimes and built-in status.
- Existing core, local inference and online transport tests remain separate targets.

## Not claimed as complete

This phase does not implement LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI or MNN-LLM adapters. Those integrations require their actual SDK dependencies, supported model packaging, ABI/delegate setup, lifecycle and generation implementations, and device testing. Remote services such as Ollama are still provider endpoint connections, not embedded engines.

## Safety
- No runtime may be loaded just because it appears in JSON.
- Native factory registration is required before `create` can return a backend.
- The existing model validation, memory admission, Guardian checks and explicit model-load flow remain in force.
