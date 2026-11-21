# Phase 28 — Third-Party Android Runtime SDK Packaging

## Included in the complete debug APK build
The main Android debug workflow now opts into these Android SDKs:
- **LiteRT-LM** — Google AI Edge Android/JVM Kotlin runtime. Official Android Gradle coordinate: `com.google.ai.edge.litertlm:litertlm-android:latest.release`.
- **ExecuTorch** — PyTorch Android AAR, pinned to `org.pytorch:executorch-android:1.0.0`. The AAR includes Java bindings and JNI runtime libraries for arm64-v8a and x86_64.
- **ONNX Runtime Mobile** — Microsoft Android AAR, coordinate `com.microsoft.onnxruntime:onnxruntime-android:latest.release`.

The SDKs are independently selectable:
```powershell
gradle clean assembleDebug -PseedxLlamaCpp=ON -PseedxRuntimeSdks=litert,executorch,onnxruntime
```

## Runtime Manager
`RuntimeSdkManager` performs non-invasive class-presence probes and labels each item as either SDK not packaged or SDK present / adapter pending. A class probe does not prove that a model loads, that the SDK can generate tokens, or that a GPU/NPU delegate is available.

## What is not represented as completed
- **LiteRT-LM:** dependency packaging is configured; the SEED-X conversation lifecycle and streaming generation bridge still needs implementation and device validation.
- **ExecuTorch:** dependency packaging is configured. Its Android LLM Java API is marked experimental by upstream; a compatible .pte model, tokenizer assets, and SEED-X lifecycle bridge are still required.
- **ONNX Runtime Mobile:** this is the general ONNX inference runtime. It is not the ONNX Runtime GenAI SDK.
- **ONNX Runtime GenAI:** the official Java API documentation states package publication is pending; source-built Android integration is a separate project.
- **MLC LLM:** Android integration requires the generated `mlc4j` Gradle subproject and model-specific compiled model libraries; it cannot be enabled by adding a single Maven coordinate.
- **MNN-LLM and ncnn:** require matching native builds, model conversion/packaging and a SEED-X adapter. They are not falsely marked as included.

## Design and safety
- One active inference engine at a time is the recommended mobile policy to control RAM and thermal load.
- SDKs do not receive device-control authority. Guardian remains the authorization boundary for tools.
- No API credentials or model weights are bundled.
- No GPU/NPU capability is claimed based only on SDK presence.
- Physical-device model load, generation, cancellation, memory and thermal tests remain required.

## Official integration references
- LiteRT-LM Android: https://developers.google.com/edge/litert-lm/android
- ExecuTorch Android: https://docs.pytorch.org/executorch/stable/using-executorch-android.html
- ONNX Runtime Android: https://onnxruntime.ai/docs/install/
- ONNX Runtime GenAI Java API: https://onnxruntime.ai/docs/genai/api/java.html
- MLC LLM Android: https://llm.mlc.ai/docs/deploy/android
