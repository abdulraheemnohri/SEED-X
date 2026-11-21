# Phase 15 — Diagnostics Center & Recovery Evidence

## Screens and controls

The existing Diagnostics screen now exposes:
- Device, Android API, network and native runtime status.
- Available memory and reserve values when the native runtime reports them.
- Active provider profile metadata (never the decrypted credential).
- Guardian emergency-stop and audit-chain verification.
- Latest crash report, bounded crash/error history, and recent app event timeline.
- Copy and GitHub issue-draft actions.

Settings includes a local breadcrumb opt-in. It is enabled by default and records screen navigation only. Disable it to stop adding new screen-navigation events.

## Local diagnostic storage

All files remain in app-private storage under `files/crash-reports`:
- `latest-crash.txt`: most recent fatal/non-fatal report.
- Timestamped `.txt` reports: up to 10 retained history entries.
- `breadcrumbs.txt`: bounded recent screen-navigation timeline (16 KiB maximum).

Crash reports include stack trace, app version, Android version/API, device manufacturer/model, timestamp and thread. Breadcrumbs contain screen names only. There is no background upload, telemetry endpoint, GitHub token, provider key, chat content or model file collection.

## Reporting workflow

1. Open Diagnostics.
2. Inspect the latest report or select an older report from history.
3. Copy it or prepare a GitHub issue draft.
4. Review and remove identifying information.
5. Submit manually on GitHub.

## Recovery / self-healing policy

This phase adds diagnostic evidence and operator review, not autonomous code repair. SEED-X does not modify its own binaries, policies, Guardian configuration or source code based on crash reports. Any future repair must be a proposed patch, tested separately, and explicitly approved by the operator.

## Known limitations

- Java uncaught exceptions are captured; native C++ crashes, ANRs and OS process kills are not reliably captured by this handler.
- Breadcrumbs are screen-navigation events, not a complete event recorder.
- Non-fatal exceptions are captured only where code explicitly calls `recordNonFatal`.
- No GitHub API write or automatic issue submission is performed.
- Device validation and CI results are required before declaring release readiness.
