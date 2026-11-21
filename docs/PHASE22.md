# Phase 22 — Android UI and Operations

## Implemented
- Added Permission Center showing the app's declared permission scope, notification permission state, and shortcuts to Android app-permission and battery settings.
- Added Task Manager for the explicit foreground agent session with start/stop controls. The service is not an always-on daemon.
- Added Model Download Manager for curated GGUF catalog entries using Android DownloadManager into app-specific external storage. Completed files can be imported into app-private model storage after GGUF magic-byte validation.
- Existing Models screen continues to import via Storage Access Framework, validate GGUF, load/unload models and delete inactive imported models.
- Added high-contrast text preference and adjustable in-app text scale (0.85x–1.5x).
- Workflow Studio supports locally saved custom workflow templates, editing custom workflows, saving built-in templates as custom copies, deleting custom workflows and manually launching them into Chat.

## Platform and safety boundaries
- No broad storage permission is used; downloads use app-specific storage and imports use SAF.
- Notification permission is requested only when the user starts a foreground session.
- No Accessibility service, overlay, SMS, call-log, contacts, location, camera, background microphone, notification listener or device-admin permission is declared.
- Task Manager currently manages the foreground session only. Persistent scheduled WorkManager jobs, pause/resume queues and unattended task execution are not implemented.
- Full light/system theme switching and a visual drag-and-drop workflow editor remain future work; the current interface uses the dark palette and manual workflow launcher.


## Additional mobile UI improvements
- Compact bottom navigation with Home, Chat, Models and Settings.
- Grouped mobile drawer menu for all screens, global screen search, screen favourites and startup-screen selection.
- Quick-launch dashboard grid and top-bar overflow actions for emergency stop, cancellation and support.
- Settings JSON export/import through Android's Storage Access Framework. The backup is allowlisted and excludes provider profiles, API keys, chat history and model files; imported values are schema- and range-validated.


## Phase 24 — Theme, command palette and dashboard controls
- Dark, light and Android-system theme selection with persisted preference and matching system bars.
- Command palette searches all app screens and exposes guarded actions for emergency stop, generation cancellation, model unload, background session controls, settings export and optional donation support.
- Home dashboard widgets can be individually shown or hidden: system overview, quick-launch grid and favourites.
- Theme and dashboard preferences are included in the allowlisted JSON settings backup; provider secrets, provider profiles, chat history and model files remain excluded.
- High-contrast text uses a theme-aware muted text color.

Limitations: this is a native Android command palette, not an arbitrary shell. It only exposes the listed fixed actions and screen routes. System theme changes are applied when Android recreates the Activity; manual theme selection is applied immediately.


## Phase 25 — Task scheduling, workflow structure and diagnostics
- Added WorkManager periodic reminders with operator-selected 1–24 hour intervals. The worker only posts a reminder notification; it never starts the agent, contacts a provider, or executes tools.
- Added local task event history for foreground-session start/stop requests and reminder schedule changes. History is bounded to 50 events and does not store prompts, responses or credentials.
- Added a visual workflow node editor: create, remove and reorder up to 20 named steps with up/down controls, inspect the saved step graph, and launch the workflow as a planning prompt in Chat. No workflow is autonomously executed.
- Workflow step lists are preserved when editing and validated during JSON backup import.
- Provider connection test time, result and elapsed milliseconds are displayed as local diagnostics; request counters remain local and do not represent provider billing.
- Added a runtime status probe showing native bridge response time, runtime catalog count, backend state and available-memory estimates. This is not an inference benchmark.
- CI now checks reminder-worker safety boundaries, workflow entry points and diagnostics hooks.

Limitations: Android WorkManager periodic delivery is inexact and may be delayed by battery optimization. Reminders require notification permission to be visible. The workflow builder is a reorderable linear node editor, not a free-form drag-and-drop graph canvas. Provider latency measures the connection test request only, not model token throughput.
