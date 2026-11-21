# Phase 27 — Complete Debug APK Packaging

## Delivered
- The primary GitHub Actions debug artifact is now built with `-PseedxLlamaCpp=ON`.
- The primary artifact is named `seedx-debug-apk` and includes the native CPU llama.cpp adapter alongside the existing Android application.
- A clean, separate JNI-stub build is published as `seedx-debug-apk-stub` for devices/build environments where the embedded inference build is not suitable.
- Both builds use `clean assembleDebug` so native build configuration does not accidentally reuse stale outputs from the other variant.
- README build/install instructions now identify the complete llama.cpp-enabled debug APK as the recommended build.

## Included in the complete debug APK
- All registered SEED-X Android screens and adaptive navigation, including Device Status and Backend Manager.
- Provider catalog and saved provider profile management; credentials are never bundled.
- Runtime inventory, live native runtime status, local GGUF model import and model management UI.
- Device battery, thermal, RAM, storage, hardware-feature and Android-version status panels.
- Guardian, policy-gated tools, crash diagnostics, workflows, task reminders, settings, backups and donation/support links.
- Embedded llama.cpp CPU inference adapter when the optional native build completes.

## Deliberate exclusions and limitations
- Model weights are not bundled; users import or download a model they are licensed to use. This keeps the APK practical in size.
- API keys and user provider profiles are not bundled. Keys remain encrypted in the app's Android Keystore-backed storage.
- LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI, MNN-LLM, ncnn, OpenVINO and TensorRT-LLM remain catalog entries until their SDK-backed adapters, packaging and device tests are completed.
- No GPU/NPU acceleration is claimed; the currently integrated llama.cpp build is CPU-only.
- The APK is a debug build, not a signed production release. Release signing and physical-device validation remain separate steps.

## CI verification
The Android debug job now builds and uploads both APK variants independently. A successful workflow run is required before describing either artifact as built. The primary artifact must be downloaded from the successful GitHub Actions run under the artifact name `seedx-debug-apk`.
