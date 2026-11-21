# Phase 14 — Crash, Error & GitHub Bug Reporting

## Implemented

- A process-wide uncaught-exception handler records fatal Java crashes locally before delegating to Android's original crash handler.
- A non-fatal reporting API records errors that the app explicitly classifies; the cloud-provider request failure path uses it.
- A bounded latest-report file is stored under app-private `files/crash-reports/latest-crash.txt`.
- Settings provides report preview, copy, GitHub issue preparation, and local deletion.
- GitHub handoff opens a prefilled issue draft in the browser. The user must review and press Submit on GitHub.

## Privacy and security

- No report is uploaded automatically.
- No GitHub token, provider API key, chat history, model files, arbitrary files, or request prompt is added to a report.
- Reports include exception stack trace, timestamp, app version, Android version/API, device manufacturer/model, and thread name.
- Device details may still be identifying. Review and edit the draft before submission.
- The latest report is bounded to 96 KiB and overwritten by the next recorded event.

## Limitations

- This is a single latest-report recorder, not a full analytics/crash-aggregation service.
- Native C++ process crashes and ANRs are not captured by the Java uncaught-exception handler.
- Non-fatal capture only happens at code paths that explicitly call `CrashReporter.recordNonFatal`.
- The GitHub handoff requires the user to be signed in to GitHub and manually submit the issue.
- A report is not automatically associated with a GitHub Actions run or CI log.

## Verification

CI compiles the Android debug APK and runs native tests. A real crash/relaunch test and GitHub issue draft review still need testing on a physical Android device.
