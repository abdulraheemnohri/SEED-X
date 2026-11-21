# SEED-X Phase 9 — Guardian-reviewed tool proposals

Version: 0.9.0 development increment

## Delivered in this increment

### 1. AI proposal review boundary
The Android Tool Center accepts a strict JSON proposal with only four fields:
- `tool`
- `operation`
- `resource`
- `payload`

Unknown fields, unknown tool IDs, invalid operation/resource combinations and oversized values are rejected before native execution. The UI presents the exact request to the operator. The operator must confirm every proposal. A `files.write/create` proposal additionally requires a native, request-bound, one-use Guardian approval ticket. The AI does not receive an approval ticket and cannot call JNI directly.

A copyable prompt instructs an AI model to return the supported JSON shape. The prompt is advisory; native registration and Guardian policy remain authoritative.

### 2. New native workspace search tool
`workspace.search / search / workspace://` is registered in the C++ ToolRegistry and has an explicit default-Allow read policy. It:
- searches only inside the app-private workspace;
- rejects absolute paths, traversal, backslashes and invalid resource paths;
- skips symbolic links;
- limits traversal depth to 3;
- inspects at most 200 files;
- skips files larger than 256 KiB;
- returns at most 50 matches;
- requires a 1–256 byte query;
- skips binary-like files containing NUL bytes.

The Android Tool Center exposes a search field and the proposal validator supports this exact registered action. The C++ policy test asserts its default read permission.

### 3. Provider catalog drives presets
The Android Provider Hub now reads `provider_catalog.json` from app assets instead of maintaining a second hard-coded list. Provider additions and access notes in the catalog are reflected in the preset list. Credentials are still entered by the operator and encrypted locally using Android Keystore.

## Existing capability boundaries

### Provider transport
The Android app currently implements non-streaming text chat adapters for:
- OpenAI-compatible chat completions (also used by compatible remote/local servers);
- Anthropic Messages;
- Gemini generateContent.

The provider catalog contains templates, not guaranteed free access. Free quotas and model availability can change. No API key is bundled.

Streaming, image/audio/video input, native tool/function calling, provider-specific structured output, and broad model discovery are not claimed as complete in this increment.

### Local runtimes
The embedded GGUF runtime is llama.cpp through the existing native C++ backend and optional build. Ollama, LM Studio, llama-server, LocalAI, vLLM and SGLang are remote/local-server connections through the compatible HTTP transport; they are not embedded runtimes.

LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI, MNN-LLM, ncnn and OpenVINO remain catalogued integration targets. Their SDKs, model conversion/packaging, ABI bridges, device delegates and device-level test coverage are not present. They must not be displayed as installed or runnable backends.

## Verification

The repository CI is the source of truth for compile and host-test status. A successful CI build does not replace physical Android device testing, runtime memory profiling, thermal testing, or model compatibility tests.

## Safety invariant

System 2 may suggest. System 1 decides. Tool execution requires an exact registered tool and matching policy. Writes require a separate operator approval ticket. No proposal can alter Guardian policies, registry contents, emergency-stop state, or invoke arbitrary shell/process execution.
