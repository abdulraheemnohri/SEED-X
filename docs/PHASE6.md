# SEED-X Phase 6 — Native Android Integration

## Delivered
- JNI bridge compiled into the Android shared native library.
- Java Android application loads seedx_android and initializes the C++ AgentKernel.
- Native runtime status reports llama.cpp availability, lifecycle state, loaded model and available memory.
- GGUF model selection calls the real C++ LocalAiRuntime loader.
- Model admission estimates RAM at 1.5x model file size plus a minimum 256 MiB runtime allowance; a 768 MiB system reserve is kept.
- Local chat routes to the loaded model before online providers.
- C++ token callbacks stream generated text to Android; UTF-8 partial token boundaries are buffered before crossing JNI.
- Stop Generation calls an atomic cancellation flag checked between generated tokens.
- Guardian screen reads live C++ emergency-stop and audit-chain status.
- Operator-confirmed Emergency Stop cancels generation and freezes Guardian authorization; reset requires a second explicit confirmation.
- Online fallback remains disabled by default and is blocked while Guardian emergency stop is active.
- Host core tests verify the kernel emergency-stop accessor and reset behavior.

## Build on Windows PowerShell

Build Android UI + JNI bridge with the default stub backend:
    gradle assembleDebug

Build with the real llama.cpp GGUF backend:
    gradle assembleDebug -PseedxLlamaCpp=ON

Install:
    adb install -r android/app/build/outputs/apk/debug/app-debug.apk

## First run
1. Open SEED-X and select Models.
2. Import a GGUF model using Android's document picker.
3. Build/install with -PseedxLlamaCpp=ON; otherwise the app reports that llama.cpp is not compiled.
4. Tap Load this model. Loading is rejected if the GGUF header is invalid or the memory budget does not fit.
5. Return to Chat. A loaded local model is selected before online providers.
6. Tap Stop generation to cancel at a token boundary.
7. Open Guardian to inspect live emergency-stop state and audit-chain verification.

## Memory guidance
The loader uses an estimate, not exact RAM consumption. On a 6 GB device, start with small quantized GGUF models and a 2K context. Close memory-heavy apps before loading. llama.cpp may still fail allocation under real runtime pressure.

## Safety boundary
- JNI exposes inference lifecycle, cancellation and read-only Guardian status plus explicit emergency-stop/reset controls.
- No JNI function executes shell commands, accessibility actions, SMS, contacts or arbitrary tools.
- Online provider requests are blocked while Guardian emergency stop is active.
- Generated model text remains untrusted and is never interpreted as a tool instruction.
- Emergency-stop reset is called only after a separate operator confirmation dialog.
- Any future device action must be represented as a typed ToolRequest and authorized by C++ Guardian immediately before execution.

## Known limitations
- JNI local inference requires the optional llama.cpp build flag. Default Android CI compiles the bridge with the unavailable-backend stub.
- Cancellation is cooperative between token-generation steps; it cannot interrupt model loading or a single long native decode call.
- Provider Java HTTP requests remain separate from the C++ Phase 4 ProviderAdapter.
- Guardian UI exposes emergency-stop state and audit verification; policy editing, approval-ticket UX and audit-log browsing remain future work.
- Android instrumentation tests and physical-device benchmarks are not yet available.
- GPU/NPU acceleration is not enabled; current llama.cpp configuration is CPU-only.

## Verification
CI compiles the Android JNI library with the default stub and runs the C++ host test suite. The optional llama.cpp backend must be built separately. No successful physical-device inference or APK installation should be assumed until tested on target hardware.
