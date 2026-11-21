# SEED-X

**Self-Evolving Android AI Agent** — a local-first native Android AI agent foundation.

## Phase 1 — Native foundation
C++20 engine, Android NDK NativeActivity shell, OpenGL ES 3 rendering surface, lifecycle kernel, event bus and host-side tests.

## Phase 2 — Guardian security & governance
- Independent default-deny policy engine with exact tool/operation and resource-prefix scopes.
- Most-specific policy matching with deny-first tie breaking.
- Candidate policy validation and in-memory replacement.
- Explicit operator approval tickets, bound to one request, short-lived and one-use.
- Emergency stop that blocks authorization and invalidates pending approvals.
- Bounded hash-linked audit trail; audit-capacity exhaustion fails closed.
- Host tests for policy, approval, emergency-stop and audit behavior.

## Roadmap
- Phase 3: local inference runtime and model manager.
- Phase 4: online AI provider adapters.
- Later: persistent memory, trusted tool runner, Android Keystore-backed policy storage, durable cryptographic audit checkpoints and controlled self-improvement.

## Repository layout
- engine/: platform-independent C++20 core and Guardian.
- android/app/: NativeActivity application and OpenGL ES UI.
- tests/: host-side core tests.
- docs/: architecture and implementation notes.

## Build on Windows 10
Install Android Studio, Android SDK Platform 35, Android NDK (Side by side), CMake 3.22.1 and JDK 17. Open the repository in Android Studio, sync Gradle, then run:

```powershell
gradle assembleDebug
adb install -r android\app\build\outputs\apk\debug\app-debug.apk
```

## Run host tests
Install CMake and a C++20 compiler (Visual Studio 2022 or Clang):

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Security notes
Phase 2 authorizes requests but does not execute tools. Unmatched actions are denied. Guardian policy is independent from the AI model. The current in-memory FNV-1a audit checksum is not cryptographic protection. Durable policy and audit persistence are later milestones.

## License
MIT. See LICENSE.
