# Phase 31 — Universal Chat, Attachments, Voice Chat, Integrations and SEED Growth

## Universal Chat

The Chat screen now includes:
- Select the currently loaded local model or a saved cloud provider profile directly from Chat.
- Explicit local/provider routing preference. Selecting a provider bypasses local auto-routing; selecting local requires a loaded local runtime.
- New chat and local chat history switching, plus per-conversation rename/delete actions.
- The selected local/provider route and provider profile ID are saved with each archived conversation.
- Up to 30 archived conversations, with up to the most recent 100 messages retained in the active conversation.
- Search, retry, copy, Markdown export, clear and long-press message actions remain available.
- Quick links to Skills, Guardian Tools, Providers, Integrations and Workflows.

Chat archive is local-only and is not included in settings backup. If history retention is disabled, SEED-X does not write new chat history to preferences.

## File attachments

- Uses Android ACTION_OPEN_DOCUMENT with multiple selection and any MIME type.
- Each selected file is copied into app-private storage; no broad storage permission is requested.
- Per-file import limit: 20 MB.
- Text-like formats (TXT, Markdown, CSV, JSON, XML, YAML, HTML and logs) are extracted up to 64 KiB per file and included as untrusted prompt context.
- Images are retained privately and sent as base64 image payloads to compatible Gemini, Anthropic and OpenAI-compatible cloud transports. Image payloads are capped at 6 MB each. The selected model must support vision; local text-only models cannot inspect them.
- Audio, video, PDF, Office documents, archives and other binary files are retained as attachment metadata and private files, but their contents are not yet decoded or sent as model payloads.

**Important capability boundary:** attaching a binary file does not mean every model can inspect it. This phase does not yet add PDF/Office parsing, audio-file transcription, video frame extraction or archive decompression. A model/runtime adapter must explicitly support the modality and payload format before SEED-X can analyze those contents. The chat UI must not claim it has read an unsupported attachment.

## Voice chat

- Push-to-talk voice chat starts Android ASR from the Chat screen.
- Recognized speech is inserted as a user message and sent through the currently selected model route.
- Optional automatic TTS playback for assistant replies.
- TTS setting is included in the allowlisted settings backup.
- No continuous hot microphone, background wake word, or automatic recording is enabled.

## Model, skills and tools

- The chat model picker selects a loaded local runtime or one of the saved provider profiles. Route selection is stored per conversation, so switching chats restores its selected route.
- Skills are existing reusable prompt templates, not arbitrary executable code.
- Tools continue to require native Guardian authorization.
- Workflows remain manually launched plans; no model-initiated autonomous execution is introduced.

## Plugins and connectors

A new Integrations screen and mirrored JSON registry describe built-in and planned connectors:
- Integrated: Puter.js WebView, Android Storage Access Framework, Guardian tools and prompt skills.
- Planned adapters: MCP HTTP, GitHub, Dropbox, WebDAV and a signed plugin runtime.
- Google Drive can be used through Android's system document picker when the provider is installed on the device.

MCP endpoint profiles can be saved with HTTPS validation, but this phase does not yet implement MCP handshake, authentication, resource discovery or tool execution. Plugin code is not dynamically loaded or executed.

## SEED Growth

The Growth screen tracks local project metrics and accepts human-reviewed improvement proposals. It does not rewrite source code, Guardian policy, signing keys or trusted update logic. Model replacement and System 1 changes must remain separate, tested and explicitly approved.

## Validation and remaining work

The CI static guard checks the new integration catalogue, screen registration, chat attachment picker, session archive, model picker, voice chat and growth UI.

Still needed:
- Android build and device testing.
- PDF and Office document text extraction.
- Native audio payload / transcription support and additional image provider adapters.
- Audio file transcription and video frame extraction.
- MCP HTTP client, auth and Guardian-scoped tool execution.
- Signed plugin manifest format, sandbox and permission review.
- Database-backed chat archive (Room) and attachment retention/cleanup policies.

Official Android file picker guidance: https://developer.android.com/training/data-storage/shared/documents-files
