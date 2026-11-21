# Phase 16 — Error Intelligence & CI Evidence

## Implemented

- Crash reports now include a deterministic short SHA-256 fingerprint derived from normalized exception/stack-frame text. Line numbers are normalized so repeated failures at the same call sites can group together.
- A lightweight local classifier labels common failures as MEMORY, PERMISSION_OR_SECURITY, NETWORK, DATA_OR_PARSING, NATIVE_BRIDGE, APPLICATION_LOGIC or UNCLASSIFIED.
- Diagnostics includes a duplicate-crash grouping view based on those fingerprints.
- Clearing diagnostics removes the latest report, retained historical reports and breadcrumb file.
- Core CI writes a GitHub Actions job summary containing job result, commit and run link, including on failure.

## Safety and privacy

Fingerprints are local diagnostic identifiers, not proof that two failures have identical root causes. Classification is heuristic and must not trigger an automatic repair. Reports remain app-private until the user copies or manually submits them. CI summaries contain only job metadata and a run link; they do not upload application logs or user data.

## Native crash and ANR limitations

This phase does not install POSIX signal handlers or claim to capture native SIGSEGV/SIGABRT, process kills, or Android ANRs. Such handlers can be unsafe in a corrupted process and need a dedicated, carefully tested crash-dump strategy. Native runtime status and JNI bridge errors can still be inspected through the existing diagnostics screen, but native crash dump collection remains future work.

## Recovery policy

No automatic source modification is performed. Future repair automation must generate a reviewable patch, run isolated tests, preserve rollback capability, and require Guardian/operator approval before application.
