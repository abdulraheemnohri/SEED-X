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
