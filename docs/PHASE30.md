# Phase 30 — Voice, universal AI capabilities and Puter.js

## Delivered

### Android voice studio

- Android SpeechRecognizer is integrated for ASR.
- The user can select on-device-only recognition or the normal system recognition service.
- On-device mode checks Android's on-device availability first. It does not silently fall back to a network recognizer.
- The RECORD_AUDIO permission is declared and requested only after the user presses Start microphone recognition.
- Recognition stops when leaving the Voice screen, pausing the Activity, receiving a final result, or pressing Stop.
- The language is configurable using a BCP-47 tag such as `en-US` or `ur-PK`.
- Android TextToSpeech supports speaking the latest assistant response, with configurable speed and pitch and a shortcut to Android's engine/voice settings.

The Android SpeechRecognizer API requires creation and lifecycle handling on the main thread and an explicit destroy call. SEED-X follows that lifecycle. On-device recognition is available only when the device's recognition service reports support.

### Voice and audio model catalogue

The separate `voice_runtime_catalog.json` describes:
- ASR: Android SpeechRecognizer, Whisper.cpp, sherpa-onnx, Vosk, Moonshine, ML Kit GenAI Speech Recognition.
- TTS: Android TextToSpeech, Piper, Kokoro, ML Kit GenAI TTS.
- VAD: Silero VAD.
- Other planned speech capabilities: speech translation, wake word, speaker diarization, audio event recognition, voice enhancement and voice conversion.

Only Android system ASR/TTS are platform-integrated in this phase. The other model/runtime entries are catalog-only or alpha. They are not falsely presented as installed inference adapters.

### Universal AI capability catalogue

The new AI capability inventory separates tasks from execution runtimes:
- Text generation, chat, reasoning and planning.
- Vision-language and image understanding.
- ASR, TTS, speech translation, VAD, wake word and speaker diarization.
- Audio event classification and speech enhancement.
- Text/image embeddings and vector retrieval.
- Reranking.
- OCR and document parsing.
- Image generation, video understanding and video generation.
- Realtime multimodal voice/vision.
- Guardian-gated agent tools, MCP and skills.

The web runtime catalogue additionally tracks WebLLM, Transformers.js, ONNX Runtime Web, MediaPipe Tasks Web and TensorFlow.js. They are not native Android runtimes and are not yet integrated into the Android app.

### Puter.js workspace

The Android screen named **Puter.js** opens a local HTML workspace in a WebView and loads the official Puter.js v2 script from its CDN. The current workspace exposes:
- AI chat.
- Available-model listing.
- Text-to-speech.
- Speech-to-text from a public HTTPS audio URL.
- Image understanding / OCR from an HTTPS image URL.
- Image generation, with test mode selected by default.

Each action asks the operator before sending content online. Puter is an online user-pays service; it is not an offline model or a free/unlimited API. No SEED-X provider API key is stored. The WebView exposes no JavaScript-to-native bridge. Links outside Puter's HTTPS domains are opened externally.

Realtime voice, speech-to-speech, video generation and direct microphone upload are not wired in this first integration.

### Settings and controls

- ASR engine preference: on-device or system.
- ASR language.
- TTS speed and pitch.
- Voice settings are included in the allowlisted JSON settings backup and are range-validated on import.
- Microphone permission is not granted automatically and is not used by the background agent service.

## Model/runtime status matrix

| Capability/runtime | Current status |
|---|---|
| Android SpeechRecognizer | Integrated platform ASR; on-device availability is device-dependent |
| Android TextToSpeech | Integrated platform TTS; installed voices determine offline behavior |
| Puter.js | Optional online WebView integration |
| LiteRT-LM / ExecuTorch | Phase 29 adapters implemented; build/device certification pending |
| Whisper.cpp / sherpa-onnx / Vosk / Moonshine | Catalog-only; adapters and model packaging required |
| Silero VAD / Piper / Kokoro | Catalog-only; adapters and model packaging required |
| ML Kit GenAI speech APIs | Alpha / supported-device dependent; not integrated in this APK |
| WebLLM / Transformers.js / ONNX Runtime Web | Catalog-only browser runtimes |

## Build and verification

Build the Android debug APK with the repository's existing full-runtime workflow. The phase adds no additional Maven dependency; it uses Android platform speech APIs and the WebView already supplied by Android.

The GitHub static guard now validates the new catalogs, their mirrored Android assets, speech permission declarations, ASR/TTS controls, Puter.js APIs and the absence of a native JavaScript bridge.

Still required:
- A successful GitHub Actions Android build.
- Test on physical Android devices with and without on-device ASR support.
- Test Urdu and English recognition availability on the actual installed speech service.
- Confirm TTS installed voices and offline availability.
- Test WebView Puter sign-in, user-pays consent, API failures and external-link handling.
- Add real Whisper.cpp and sherpa-onnx adapters in a later milestone, then VAD/TTS adapters and audio model download/licence management.

## Official references

- Puter.js AI: https://docs.puter.com/AI/
- Android SpeechRecognizer: https://developer.android.com/reference/android/speech/SpeechRecognizer
- ML Kit GenAI Speech Recognition: https://developers.google.com/ml-kit/genai/speech-recognition/android
- ML Kit GenAI Text-to-Speech: https://developers.google.com/ml-kit/genai/text-to-speech/android
