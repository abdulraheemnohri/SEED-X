# Phase 1 implementation notes

## Scope
Phase 1 delivers the Android native application shell and a platform-independent C++20 core. It deliberately does not execute AI-generated tools.

## Runtime lifecycle
`AgentKernel` exposes Stopped, Initializing, Ready, Running, Paused and Error states. Android lifecycle callbacks pause/resume the kernel and release/recreate EGL resources when the native window is destroyed or created.

## Guardian policy
The Phase 1 policy is intentionally hard-coded and default-deny:
- Allow `memory.read` with operation `read`.
- Allow `system.status` with operation `read`.
- Deny all other operations.
- Guardian freeze denies all requests.

This is a safe bootstrap policy, not the final signed policy store.

## UI
The current OpenGL ES 3 surface is a visual shell with a status stripe and three dashboard panels. It proves native window, EGL context and render-loop integration. Text, navigation, touch controls, accessibility and production dashboard widgets are scheduled for the UI milestone.

## Known Phase 1 limitations
- No model runtime or provider networking.
- No persistent SQLite storage yet.
- No autonomous task executor.
- No native binary self-update.
- No foreground service or guaranteed background execution.
- No Android Keystore integration yet.
- The native UI shell currently consumes touch events without exposing interactive controls.

## Verification
Run host tests with CMake and CTest as described in the root README. Android APK compilation requires the Android SDK/NDK toolchain and must be verified on an Android device or emulator.
