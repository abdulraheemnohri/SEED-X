# SEED-X

**Self-Evolving Android AI Agent** — a local-first native Android AI agent foundation.

## Phase 1: Native foundation

Phase 1 establishes the C++20 core, Android NDK NativeActivity shell, OpenGL ES 3 rendering surface, lifecycle kernel, default-deny Guardian and host-side tests.

### Included
- C++20 platform-independent engine library.
- Android NativeActivity entry point using NDK native app glue.
- Native OpenGL ES 3 dashboard shell.
- Agent lifecycle state machine.
- Guardian authorization with default-deny behavior.
- Thread-safe event subscription and publication.
- Host CMake test executable.
- Android Gradle project targeting API 35 / minimum API 31.

### Not included yet
- Local model inference (Phase 3).
- Online provider adapters (Phase 4).
- Persistent conversation and semantic memory.
- Tool execution or shell access.
- Autonomous self-evolution.

## Repository layout
- `engine/`: platform-independent C++20 core.
- `android/app/`: NativeActivity application and OpenGL ES UI.
- `tests/`: host-side core tests.
- `docs/`: architecture and build documentation.

## Build on Windows 10
Install Android Studio, Android SDK Platform 35, Android NDK (Side by side), CMake 3.22.1 and JDK 17. Open the repository in Android Studio, sync Gradle, then run:

```powershell
gradle assembleDebug
adb install -r android\app\build\outputs\apk\debug\app-debug.apk
```

The Android Studio Gradle tool window can run the same `assembleDebug` task. A Gradle wrapper can be generated locally from the installed Gradle distribution.

## Run host tests
Install CMake and a C++20 compiler (Visual Studio 2022 or Clang):

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Security boundary
Phase 1 is a foundation, not an autonomous agent. The Guardian is default-deny. Only the explicit read-only capabilities `memory.read` and `system.status` are allowed when the operation is `read`. There is no shell executor, model downloader, remote API client or self-modification mechanism in this phase.

## License
MIT. See [LICENSE](LICENSE).
