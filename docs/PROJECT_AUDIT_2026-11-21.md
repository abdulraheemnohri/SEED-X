# SEED-X Complete Project Audit
Date: 2026-11-21
Reviewed branch: main
Reviewed baseline: edae5d4c97c5ce516b3e330d7020da2c7bdf8b49
Audit scope: repository tree, Android app, JNI bridge, native C++ engine, catalogs, CI workflows, tests, security policy and phase documentation.

## Executive assessment

SEED-X is a working **local-first Android AI workspace foundation with a Guardian-governed manual tool registry**. It is not yet a complete autonomous/self-evolving agent, nor a universal embedded AI runtime. The repository is compact (112 tracked tree entries at review time), has a single Android Activity as its main UI surface, and includes native host tests and Android build jobs.

The project deliberately distinguishes catalog metadata from executable runtime integrations. That is correct and should be preserved.

## What is present

- Android Java application, API 31+ minimum, target/compile SDK 35, arm64-v8a and x86_64 native ABIs.
- C++20 engine with AgentKernel, policy engine, Guardian, audit log, local inference abstraction, provider abstraction, runtime registry and typed tool registry.
- Optional pinned llama.cpp backend for GGUF; default Android build uses the stub backend.
- JNI bridge for native initialization, model loading, generation, cancellation, Guardian status, policy/audit snapshots and tool calls.
- Android provider profile UI and Java transports for OpenAI-compatible chat completions, Anthropic Messages and Gemini generateContent.
- Keystore AES-GCM credential storage; opt-in cloud routing and explicit per-send confirmation.
- Catalogs for provider templates, models, runtimes, skills, tools and workflows.
- User-started foreground service, local crash reports, SAF model/settings import-export, task reminders, and local event history.
- CI jobs for native tests, llama.cpp build, Android debug APK and unsigned release APK/AAB.

## Confirmed limitations — do not advertise as completed

1. **Embedded runtimes:** only llama.cpp is integrated as an embedded inference backend, and only in builds that enable `seedxLlamaCpp=ON`. LiteRT-LM, MLC LLM, ExecuTorch, ONNX Runtime GenAI and MNN remain adapter-missing catalog entries.
2. **Default APK:** the standard build uses the JNI stub backend. It does not provide local GGUF inference unless the optional llama.cpp build is used.
3. **Agent autonomy:** there is no autonomous agent loop or model-initiated tool planner. The user manually launches tools; workflows are prompt plans. WorkManager reminders only notify.
4. **Guardian scope:** Guardian authorizes registered tool operations and blocks tool execution during emergency stop. It is not a general Android OS permission broker and does not independently govern ordinary text generation.
5. **Providers:** three transport families exist (OpenAI-compatible, Anthropic, Gemini). The many provider catalog entries are templates/compatibility profiles, not separately implemented provider SDKs. Java Android transport and C++ provider adapter are separate code paths.
6. **Hardware acceleration:** no integrated GPU/NPU acceleration or device benchmark is claimed.
7. **Model integrity:** GGUF magic/header validation exists, but cryptographic SHA-256 verification and signed model provenance are not implemented.
8. **Android validation:** there are host-side C++ tests and CI APK builds, but no Android instrumentation/UI test source is present in the repository tree. Physical-device regression and TalkBack review remain outstanding.
9. **Build ergonomics:** no Gradle wrapper is checked in; a compatible Gradle installation is required.
10. **Audit evidence:** the current native audit chain uses FNV-1a as a lightweight integrity checksum, not a cryptographic MAC/signature or durable tamper-proof log.
11. **Architecture debt:** most Android UI, navigation, provider networking, persistence and workflow logic lives in one large `MainActivity.java`. This raises regression risk and makes feature-level testing difficult.
12. **Documentation drift:** earlier Phase 25 notes described themes, task scheduling and visual workflow capabilities as future work even after later commits implemented them. Phase notes need to reflect actual state.

## Findings fixed during this review

- Foreground service state could remain `running=true` after the service stopped. The stop path and `onDestroy()` now clear the flag, and the flag is set only after foreground promotion succeeds.
- Provider connection test classification could treat an HTTP error response such as “Provider returned HTTP 401” as success. It now reports success only when the response begins with the explicit success message.
- CI static regression checks were added for both fixes.

## Security observations

- The app manifest declares only INTERNET, ACCESS_NETWORK_STATE, FOREGROUND_SERVICE, FOREGROUND_SERVICE_DATA_SYNC and POST_NOTIFICATIONS; broad Accessibility, overlay, SMS, contacts, location, microphone, camera, notification-listener and all-files permissions are absent.
- Model downloads use HTTPS and app-specific storage; imported models are not cryptographically authenticated.
- `usesCleartextTraffic=true` is set at the application level to support explicitly opted-in local HTTP endpoints. The Java provider code applies a host allowlist and disables redirects, but the manifest setting itself is broader than the app's intended policy. A future hardening pass should isolate local HTTP transport and make the platform-level cleartext policy as restrictive as possible without breaking supported LAN use.
- The C++ workspace tools enforce relative workspace paths, canonical-root checks, bounded reads/search and no-overwrite writes. Continue adding symlink/race-condition tests around filesystem operations.
- Crash reports are local and user-reviewed before sharing; device model and stack traces can still be personal diagnostic data, so retain explicit review before upload.

## Recommended priority order

### P0 — release correctness
- Complete CI for the latest commit and verify APK/AAB artifacts.
- Add Android instrumentation smoke tests for startup, navigation, settings import/export, notification permission and provider form validation.
- Run the app on physical Android 12–15 devices; test low-memory behavior, background limits, notification denial and process death.
- Verify that notification icon rendering and service state recover correctly after process recreation.

### P1 — security and reliability
- Replace FNV audit checksum with a cryptographic hash chain and durable bounded persistence; document that local device storage is not tamper-proof.
- Add SHA-256 model verification against trusted, user-visible expected digests.
- Add DNS/IP resolution checks for local HTTP endpoints, including all resolved addresses, and defend against DNS rebinding.
- Add JNI bounds/error tests, native crash/ANR strategy, and filesystem race/symlink tests.
- Make provider-test HTTP failures, timeouts, cancellation and retries observable and correctly classified.

### P2 — maintainability
- Split MainActivity into screen controllers, provider transport, repository/storage, workflow editor and settings/backup modules.
- Add a Gradle wrapper and dependency/version verification.
- Add schema versioning/migrations for preferences, workflow data and provider profiles.
- Add accessibility labels, TalkBack checks, responsive tablet layouts and localization.

### P3 — roadmap integrations
- Integrate one additional embedded runtime at a time (LiteRT-LM or ONNX Runtime GenAI first), with ABI/delegate coverage and actual on-device inference tests.
- Add a persistent task queue only after task lifecycle, cancellation, Guardian approval and recovery semantics are specified.
- Add a real node/edge workflow execution engine only after every node type has a typed schema, explicit Guardian authorization and replay-safe execution.
- Keep model self-upgrade and self-modification outside the trusted Guardian control plane.

## Verification status

The audit reviewed source and CI configuration. It did not execute the app on a physical device. CI at the review point had successful native tests, llama.cpp build and unsigned release artifact jobs on a nearby prior commit; the latest Android debug job was still in progress. Treat final status as pending until the latest workflow run finishes.


## Post-audit implementation addendum
The following features were added after the baseline source review:
- Prompt Library with six built-in prompt templates and up to 50 custom templates. Template selection only fills the composer; sending remains a deliberate user action.
- Chat search, long-press message actions (copy/delete, speak assistant messages, resend user messages), retry-last-user, copy transcript, Markdown export and confirmation-protected history clearing.
- Storage Manager with app-owned model/download size estimates and local cleanup controls for chat, memory and diagnostic reports.
- Read-only Network Diagnostics for active transport, validated connectivity, metering/roaming and active provider endpoint. It does not scan the LAN.
- About & Support screen with repository, docs, issue and optional donation links.
- Settings backup includes prompt templates and dashboard visibility preferences, while continuing to exclude provider profiles, API keys, chat history and model files.
- Android 15+ dataSync foreground-service timeout callback now stops the session service and clears its running state.

These additions are subject to the latest CI run. They have not been physically tested on an Android device. The audit's runtime, security and instrumentation-test limitations remain in force.


## Phase 26 follow-up

The current main branch adds two workspace screens:
- **Device Status** reports Android identity, supported ABIs, logical CPU count, battery charge/state/source/health/temperature/voltage, battery-saver state, app battery-optimization exemption, supported thermal status, RAM pressure, storage capacity, app-owned model/download sizes and selected hardware feature declarations.
- **Backend Manager** separates embedded runtime state from saved local/LAN/cloud provider profiles, exposes online fallback and private-LAN HTTP controls, and links to Provider Hub, Runtime Manager, Models and Network Diagnostics.

No new Android permissions were added. These are read-only status integrations, not device-control features. The provider catalog remains a finite set of presets plus custom OpenAI-compatible endpoint support. Only the optional llama.cpp build is currently integrated as an embedded LLM backend; other runtime catalog entries remain unavailable until adapters are implemented. See [PHASE26.md](PHASE26.md).
