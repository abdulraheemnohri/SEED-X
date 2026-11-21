# SEED-X Phase 7 — Guardian-Gated Tool Execution

Version 0.7.0

## Objective

Add a typed tool execution layer between the Android app and the native Guardian. Tool execution is registry-based and allowlisted. Model text is never treated as executable code, shell commands, or a tool payload without an explicit operator action.

## Architecture

Android Tool Center -> JNI -> C++ ToolRegistry -> AgentKernel::authorizeAndRun -> Guardian policy + approval + audit -> registered handler -> result.

The Guardian holds its authorization mutex across each short registered handler. Emergency stop sets the frozen flag immediately and waits for an already-authorized short handler to leave that critical section. After emergency stop returns, no new tool handler can start. Handlers must remain bounded and must not call back into Guardian.

## Registered tools

| Tool | Operation | Resource | Default policy | Behavior |
|---|---|---|---|---|
| system.status | read | system:// | Allow | Reports kernel stop state and whether a local model is loaded |
| files.list | read | workspace:// | Allow | Lists up to 500 entries in the private workspace |
| files.read | read | workspace://relative/path | Allow | Reads regular files up to 1 MiB |
| files.write | create | workspace://relative/path | RequireApproval | Creates a new note up to 64 KiB; never overwrites |

All other tool identifiers and operations are denied because there is no registered handler. A policy allow rule alone cannot create a missing handler.

## Guardian policy and approval

- Tool requests contain exact tool ID, operation and resource URI.
- The registry checks for an exact tool/operation handler and matching resource prefix.
- Guardian evaluates policy immediately before the handler runs.
- files.write uses a 60-second, one-use approval ticket bound to the exact tool, operation and resource.
- The Android operator must approve a confirmation dialog before a ticket is requested.
- Missing, expired, replayed or mismatched tickets do not run the handler.
- The ticket is consumed inside the same Guardian critical section as the action.
- Emergency stop blocks new execution and clears pending approvals.
- The Android permission dashboard displays the current native policy snapshot.

## Workspace sandbox

- Root: app-private files/workspace.
- Relative paths only; absolute paths, . and .. path components are rejected.
- Resolved paths must remain inside the canonical workspace root.
- Symbolic links are skipped by directory listing; read targets must resolve inside the workspace.
- Listing is capped at 500 entries.
- Reads are capped at 1 MiB.
- Note content is capped at 64 KiB.
- Note creation requires the parent directory to exist.
- Note creation uses exclusive file creation (O_EXCL) and mode 0600; existing files are never overwritten.
- A partial write is removed on failure.

## Audit

- Every Guardian authorization decision and approval consumption is recorded in the existing bounded hash-linked audit log.
- Android can inspect the latest 100 entries from the Guardian screen or Tool Center.
- Audit-chain verification status is shown alongside emergency-stop status.
- The existing audit hash is FNV-1a for accidental-change/tamper-evidence within the chain, not a cryptographic signature or remote-proof mechanism.

## Android UI

- Adds a Tools bottom tab.
- Shows the registered tool catalog.
- Offers device/runtime status, workspace listing and bounded text-file reading.
- Note creation requires a separate explicit approval dialog.
- Guardian screen shows live policy rules, emergency-stop state and audit integrity.
- Audit viewer displays recent tool and policy events.

## Build on Windows PowerShell

Host tests:
    cmake -S . -B build
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

Android debug APK:
    gradle assembleDebug

Android debug APK with local llama.cpp:
    gradle assembleDebug -PseedxLlamaCpp=ON

Install:
    adb install -r android/app/build/outputs/apk/debug/app-debug.apk

## Verification

The core test suite covers:
- Registered handler execution.
- Duplicate handler rejection.
- Default-deny behavior for unregistered tools.
- RequireApproval decisions when no ticket is supplied.
- One-use ticket consumption and replay rejection.
- Emergency-stop blocking of registered handlers.
- Audit-chain verification.

GitHub Actions compiles the C++ core, optional llama.cpp backend and Android debug APK. CI does not prove physical-device behavior. Run the APK on an Android 12+ device and test local model loading, workspace boundaries and emergency-stop behavior before using it with important files.

## Explicit limitations

- No shell, arbitrary process execution, ADB, AccessibilityService, SMS, contacts, call, notification or unrestricted storage tools are registered.
- No device setting toggles or privileged hardware actions are exposed.
- The model does not autonomously invoke tools. The operator selects a tool in the Android Tool Center; future AI planning must still produce typed requests and pass the same Guardian gate.
- Tool handlers are built into the app; third-party dynamic plugin loading and remote plugin installation are not implemented.
- Policy editing is not exposed in the Android UI. The current policy dashboard is read-only; policy replacement remains an operator-confirmed native API.
- Audit snapshots are local and bounded; they are not remotely backed up or cryptographically signed.
- File operations are intentionally limited to the app-private workspace.
- No physical-device or performance benchmark is claimed.

## Safety invariant

System 2 proposes text. The Tool Center creates a typed request. System 1 validates the registered handler, checks policy, consumes approval if required, records the decision, and only then invokes the handler. System 2 cannot alter Guardian policy, mint approval tickets, reset emergency stop, or execute arbitrary code.
