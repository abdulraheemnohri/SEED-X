# Phase 25 — Mobile UI, Workflows and Task Reminders

## Delivered
- Dark, light and Android-system theme selection with persisted preference.
- Searchable mobile screen palette, grouped navigation menu, startup destination and favourite screens.
- Home dashboard controls for overview, quick launch and favourites.
- Validated JSON settings backup and restore; provider profiles, API keys, chat history and model files are excluded.
- Local task event history bounded to 50 events.
- User-configured WorkManager periodic reminder (1–24 hours). Reminder worker posts a notification only; it does not start the agent, contact providers or execute tools.
- Linear workflow node editor with add, remove, reorder and inspect operations (up to 20 steps). Workflows are plans and are not autonomously executed.
- Provider test timestamp/result/elapsed time and local usage counters.
- Native runtime status probe. This is not an inference throughput benchmark.

## Safety and limitations
- Android may delay periodic reminders; notification visibility requires notification permission.
- The workflow editor is a reorderable linear node list, not a free-form drag-and-drop graph canvas.
- No autonomous agent loop, scheduled tool execution, provider-triggered background execution or Android device-control automation is implemented.
- Device-specific responsive layout and TalkBack/manual accessibility review remain outstanding.

## Additional workspace expansion
- Added Prompt Library with six built-in prompt starters and up to 50 custom local templates. Choosing a template only fills the chat composer; the user must press Send.
- Added conversation search, Markdown export through Android's document picker, copy-latest-response and confirmation-protected history clearing.
- Added Storage Manager for app-owned model/download size estimates and controlled cleanup of local chat, memories and diagnostic reports. It does not scan or modify external storage.
- Added read-only Network Diagnostics for active transport, validated connectivity, metering/roaming capability and current provider endpoint; it does not scan LAN devices or generate network traffic.
- Added About screen with repository, documentation, issue-reporting and optional donation links.
- Settings backup now includes custom prompt templates and dashboard widget preferences, with schema validation. Provider profiles, credentials, chat history and model files remain excluded.

- Expanded chat message controls: long-press a message to copy/delete it, speak assistant messages, or resend user messages; retry the latest user message and copy the full transcript.

- Added Android 15+ foreground-service timeout handling. The user-started dataSync session now clears its running state and stops itself when Android invokes the platform timeout callback.

## Adaptive UI expansion
- Replaced the fixed phone-only shell with width-aware navigation: a five-item bottom bar on compact windows and a persistent grouped side rail on large windows.
- Added automatic layout selection plus user overrides for automatic, bottom-bar and side-rail navigation.
- Added comfortable/compact content density, responsive dashboard quick-launch columns (2/3/4), clearer active navigation states and consistent touch targets.
- Added resizable-activity and configuration handling so rotation, foldable/window-size changes rebuild the adaptive shell.
- Improved light-theme system-bar icon contrast, card spacing, control contrast and screen-reader content descriptions for navigation actions.
- Navigation mode and density are included in the validated settings backup.
- Device screenshots, TalkBack traversal and multi-device manual verification remain outstanding; the current implementation is native Android Views, not a Compose redesign.
