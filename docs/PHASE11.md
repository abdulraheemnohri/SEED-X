# SEED-X Phase 11 — Model format contracts and backend factory validation

## Delivered

### Explicit model format taxonomy
The engine now distinguishes GGUF, ONNX, LiteRT, MLC, ExecuTorch, MNN and ncnn package formats. `modelFormatName()` provides a stable display label.

### Honest model validation
The current file validator remains GGUF-specific and checks the GGUF magic header. Other package formats are rejected with a format-specific explanation rather than being accepted as arbitrary files. This prevents a model import screen from implying that a package can be loaded simply because its extension or catalog entry is known.

### Runtime/backend contract hardening
- An Available runtime registration must declare at least one supported format and provide a factory.
- RuntimeRegistry::create verifies that the factory returns a backend and that the backend supports at least one declared format.
- An unavailable runtime cannot be instantiated.
- Added tests for a factory whose backend format does not match its runtime declaration.
- Added model tests proving ONNX is identified but not accepted by the GGUF-only validator.

## Current support matrix

| Runtime | Package format | Status |
|---|---|---|
| llama.cpp | GGUF | Integrated when optional build is enabled |
| ONNX Runtime GenAI | ONNX | Adapter and package validator not implemented |
| LiteRT-LM | LiteRT | Adapter and package validator not implemented |
| MLC LLM | MLC package | Adapter and package validator not implemented |
| ExecuTorch | .pte | Adapter and package validator not implemented |
| MNN-LLM | MNN package | Adapter and package validator not implemented |
| ncnn | param/bin | Adapter and package validator not implemented |

## Not claimed
This phase does not add ONNX Runtime GenAI binaries, model conversion, tokenizer implementation, Android execution providers, or generation. Those are required before ONNX can be marked runnable.

## Safety invariant
A format name is not proof of runtime support. A runtime is executable only when it has a compiled factory, and that factory returns a backend that actually supports one of the formats declared by the runtime.
