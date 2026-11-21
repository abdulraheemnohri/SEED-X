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
