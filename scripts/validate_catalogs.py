#!/usr/bin/env python3
"""Validate bundled JSON catalogs and enforce the Android permission boundary."""
import json
import pathlib
import sys
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]
CATALOGS = [
    "config/provider_catalog.json",
    "config/model_catalog.json",
    "config/runtime_catalog.json",
    "config/voice_runtime_catalog.json",
    "config/ai_capability_catalog.json",
    "config/web_ai_provider_catalog.json",
    "config/integration_catalog.json",
    "config/skills.json",
    "config/tools.json",
    "config/workflows.json",
]
for relative in CATALOGS:
    path = ROOT / relative
    with path.open(encoding="utf-8") as handle:
        data = json.load(handle)
    if not isinstance(data, dict):
        raise SystemExit(f"{relative}: root must be an object")
    if relative.startswith("config/"):
        bundled = ROOT / "android/app/src/main/assets" / pathlib.Path(relative).name
        if bundled.exists() and json.loads(bundled.read_text(encoding="utf-8")) != data:
            raise SystemExit(f"{relative}: Android asset catalog is out of sync")
    if relative.endswith("provider_catalog.json"):
        providers = data.get("providers", [])
        ids = [item.get("id") for item in providers]
        if not providers or len(ids) != len(set(ids)):
            raise SystemExit("provider_catalog.json: provider list is empty or has duplicate IDs")
        for item in providers:
            for key in ("id", "name", "kind", "baseUrl", "defaultModel"):
                if not item.get(key):
                    raise SystemExit(f"provider_catalog.json: provider missing {key}")
    if relative.endswith("model_catalog.json"):
        for item in data.get("models", []):
            if not item.get("repository") or not item.get("file", "").lower().endswith(".gguf"):
                raise SystemExit("model_catalog.json: downloadable model must define a repository and GGUF filename")
    print(f"OK JSON {relative}")

manifest = ET.parse(ROOT / "android/app/src/main/AndroidManifest.xml").getroot()
android_ns = "{http://schemas.android.com/apk/res/android}"
permissions = {
    node.attrib.get(android_ns + "name", "")
    for node in manifest.findall("uses-permission")
}
for forbidden in (
    "android.permission.BIND_ACCESSIBILITY_SERVICE",
    "android.permission.SYSTEM_ALERT_WINDOW",
    "android.permission.READ_SMS",
    "android.permission.READ_CALL_LOG",
    "android.permission.READ_CONTACTS",
    "android.permission.ACCESS_FINE_LOCATION",
    "android.permission.ACCESS_COARSE_LOCATION",
    "android.permission.MANAGE_EXTERNAL_STORAGE",
    "android.permission.BIND_NOTIFICATION_LISTENER_SERVICE",
    "android.permission.BIND_DEVICE_ADMIN",
    "android.permission.CAMERA",
):
    if forbidden in permissions:
        raise SystemExit(f"High-risk permission requires security review: {forbidden}")
if "android.permission.RECORD_AUDIO" not in permissions:
    raise SystemExit("Voice feature must declare RECORD_AUDIO and request it at runtime")
if "android.speech.RecognitionService" not in {
    node.attrib.get(android_ns + "name", "") for node in manifest.findall("./queries/intent/action")
}:
    raise SystemExit("SpeechRecognizer package visibility query is missing")
print("OK Android manifest permission boundary; microphone is a user-started capability")

# Guard the mobile navigation shell against accidental removal of entry points.
activity = (ROOT / "android/app/src/main/java/org/seedx/agent/MainActivity.java").read_text(encoding="utf-8")
required_ui = (
    "private void showMobileMenu",
    "private void showScreenSearch",
    "private void chooseStartupScreen",
    "private void manageFavourites",
    "Guardian emergency stop",
    "Stop current generation",
    "Open full mobile menu",
)
for marker in required_ui:
    if marker not in activity:
        raise SystemExit(f"MainActivity is missing mobile UI entry point: {marker}")
for screen in ("Home", "Chat", "Voice", "Puter.js", "AI Capabilities", "Memory", "Prompts", "Models", "Downloads", "Runtimes",
               "Providers", "Skills", "Workflows", "Tasks", "Permissions", "Tools",
               "Guardian", "Diagnostics", "Network", "Storage", "About", "Settings"):
    if f'"{screen}"' not in activity:
        raise SystemExit(f"MainActivity is missing registered screen: {screen}")
print("OK mobile navigation and screen entry points")

# Guard safe reminder scheduling and the structured workflow builder.
gradle = (ROOT / "android/app/build.gradle").read_text(encoding="utf-8")
worker = (ROOT / "android/app/src/main/java/org/seedx/agent/SessionReminderWorker.java").read_text(encoding="utf-8")
for marker in ("androidx.work:work-runtime",):
    if marker not in gradle:
        raise SystemExit(f"Android build is missing required dependency: {marker}")
for marker in ("class SessionReminderWorker", "POST_NOTIFICATIONS", "No work was run automatically", "return Result.success()"):
    if marker not in worker:
        raise SystemExit(f"Reminder worker is missing safety behavior: {marker}")
for forbidden in ("startAgentSession(", "nativeExecuteTool(", "requestProvider("):
    if forbidden in worker:
        raise SystemExit(f"Reminder worker must not perform autonomous actions: {forbidden}")
for marker in ("scheduleTaskReminder", "recordTaskEvent", "openVisualWorkflowBuilder", "workflowStepInput", "Collections.swap(nodes", "showWorkflowGraph", "showRuntimeDiagnostics", "provider_last_test_time"):
    if marker not in activity:
        raise SystemExit(f"MainActivity is missing phase feature: {marker}")
print("OK task reminders, workflow builder and runtime/provider diagnostics")

# Regression checks for the full-project audit findings.
service = (ROOT / "android/app/src/main/java/org/seedx/agent/AgentForegroundService.java").read_text(encoding="utf-8")
for marker in ("running = false;", "running = true;", "onDestroy()", "onTimeout(int startId, int foregroundServiceType)", "stopSelf(startId)"):
    if marker not in service:
        raise SystemExit(f"Foreground service state lifecycle is missing: {marker}")
if "boolean ok=result.startsWith(\"Provider connection succeeded.\");" not in activity:
    raise SystemExit("Provider health test must classify only explicit success responses as success")
print("OK foreground-service lifecycle and provider-test result classification")

# Additional workspace screen and conversation utilities.
for marker in ("private void prompts()", "private void createPromptTemplate()", "private void searchConversation()", "private void startChatExport()", "private void exportChatTo(Uri uri)", "private void storageManager()", "private void networkDiagnostics()", "private void about()"):
    if marker not in activity:
        raise SystemExit(f"MainActivity is missing workspace feature: {marker}")
if "private static final int EXPORT_CHAT=4106;" not in activity:
    raise SystemExit("Conversation export activity result code is missing")
if "Clear conversation" not in activity or "Search conversation" not in activity or "Copy latest assistant response" not in activity:
    raise SystemExit("Chat history controls are incomplete")
for marker in ("showMessageActions", "retryLastUserMessage", "copyFullConversation", "Resend this message"):
    if marker not in activity:
        raise SystemExit(f"Chat message action is missing: {marker}")
print("OK prompt library, chat search/export, storage, network and about screens")


# Device status and backend management entry points.
for marker in (
    'case "Device":deviceStatus();break;',
    'case "Backends":backends();break;',
    'private void deviceStatus()',
    'private void backends()',
    'RUNTIME INVENTORY',
    'Copy runtime details',
    'Manage backend connections',
    'BatteryManager.EXTRA_LEVEL',
    'getCurrentThermalStatus()',
    'getAvailableBytes()',
    'Allow cloud/remote provider fallback',
    'Allow trusted private-LAN HTTP endpoints',
    'DECLARED PERMISSIONS • LIVE PACKAGE STATUS',
    'getPackageInfo(getPackageName()',
):
    if marker not in activity:
        raise SystemExit(f"MainActivity is missing device/backend feature: {marker}")
print("OK Android device status and backend manager")

# Adaptive Android workspace: phone bottom bar, large-screen navigation rail and density settings.
for marker in ("private boolean useSideNavigation()", "private void addSidebar(", "private void shell()", "onConfigurationChanged(android.content.res.Configuration", "ui_navigation_mode", "ui_density", "screenWidthDp()>=900?4:screenWidthDp()>=600?3:2", "android:resizeableActivity=\"true\""):
    if marker not in activity and marker not in (ROOT / "android/app/src/main/AndroidManifest.xml").read_text(encoding="utf-8"):
        raise SystemExit(f"Adaptive UI feature is missing: {marker}")
print("OK adaptive navigation, responsive dashboard and display preferences")

# Keep the primary debug artifact aligned with the all-in-one build contract.
workflow = (ROOT / ".github/workflows/phase1.yml").read_text(encoding="utf-8")
for marker in (
    "gradle clean assembleDebug -PseedxLlamaCpp=ON",
    "name: seedx-debug-apk",
    "gradle clean assembleDebug -PseedxLlamaCpp=OFF",
    "name: seedx-debug-apk-stub",
):
    if marker not in workflow:
        raise SystemExit(f"Android debug packaging contract is missing: {marker}")
print("OK complete llama.cpp debug APK and lightweight stub fallback artifacts")

# Verify optional third-party Android runtime SDK packaging and the runtime screen probe.
gradle_app = (ROOT / "android/app/build.gradle").read_text(encoding="utf-8")
runtime_probe = (ROOT / "android/app/src/main/java/org/seedx/agent/RuntimeSdkManager.java").read_text(encoding="utf-8")
for marker in (
    'com.google.ai.edge.litertlm:litertlm-android',
    'org.pytorch:executorch-android',
    'com.microsoft.onnxruntime:onnxruntime-android',
):
    if marker not in gradle_app:
        raise SystemExit(f"Android runtime SDK dependency is missing: {marker}")
for marker in ("LiteRT-LM", "ExecuTorch", "ONNX Runtime Mobile", "Class.forName", "adapter-pending"):
    if marker not in runtime_probe:
        raise SystemExit(f"Runtime SDK probe is missing: {marker}")
print("OK optional LiteRT-LM, ExecuTorch and ONNX Runtime Android SDK packaging")








# Phase 29 real local inference adapter guards.
root_gradle = (ROOT / "build.gradle").read_text(encoding="utf-8")
app_gradle = (ROOT / "android/app/build.gradle").read_text(encoding="utf-8")
litert_adapter = (ROOT / "android/app/src/main/java/org/seedx/agent/LiteRtLmAdapter.kt").read_text(encoding="utf-8")
et_adapter = (ROOT / "android/app/src/main/java/org/seedx/agent/ExecuTorchAdapter.java").read_text(encoding="utf-8")
adapter_contract = (ROOT / "android/app/src/main/java/org/seedx/agent/InferenceAdapter.java").read_text(encoding="utf-8")
adapter_manager = (ROOT / "android/app/src/main/java/org/seedx/agent/RuntimeAdapterManager.java").read_text(encoding="utf-8")
for marker in ("org.jetbrains.kotlin.android",):
    if marker not in root_gradle or marker not in app_gradle:
        raise SystemExit("Kotlin Android plugin is required for the LiteRT-LM adapter")
for marker in ("litertlm-android:0.16.1", "kotlinx-coroutines-android"):
    if marker not in app_gradle:
        raise SystemExit(f"LiteRT-LM dependency is missing: {marker}")
for marker in ("EngineConfig(", "candidate.initialize()", "sendMessageAsync", "cancelProcess()", "oldConversation?.close()", "oldEngine?.close()"):
    if marker not in litert_adapter:
        raise SystemExit(f"LiteRT-LM adapter lifecycle is missing: {marker}")
for marker in ("LlmModule", "LlmCallback", '"load"', '"generate"', '"stop"'):
    if marker not in et_adapter:
        raise SystemExit(f"ExecuTorch LLM adapter is missing: {marker}")
for marker in ("interface InferenceAdapter", "void generate(", "void cancel()", "void unload()"):
    if marker not in adapter_contract:
        raise SystemExit(f"Shared inference contract is missing: {marker}")
for marker in ('endsWith(".litertlm")', 'endsWith(".pte")', "requireMemoryHeadroom", "generationExecutor", "active.unload()"):
    if marker not in adapter_manager:
        raise SystemExit(f"Runtime selection or memory management is missing: {marker}")
for marker in ("PICK_EDGE_MODEL", "PICK_EDGE_TOKENIZER", "sendThirdPartyLocal", "runtimeAdapters.isLoaded()", "Import LiteRT-LM / ExecuTorch model", "Stop current generation"):
    if marker not in activity:
        raise SystemExit(f"Android UI integration for third-party inference is missing: {marker}")
print("OK Phase 29 LiteRT-LM/ExecuTorch adapters, model import, streaming and runtime selection")


# Phase 30: voice runtime, AI capability inventory and Puter.js workspace.
voice_catalog = json.loads((ROOT / "config/voice_runtime_catalog.json").read_text(encoding="utf-8"))
ai_catalog = json.loads((ROOT / "config/ai_capability_catalog.json").read_text(encoding="utf-8"))
web_catalog = json.loads((ROOT / "config/web_ai_provider_catalog.json").read_text(encoding="utf-8"))
voice_ids = {item.get("id") for item in voice_catalog.get("runtimes", [])}
for required in ("android-speech-recognizer", "android-tts", "whisper-cpp", "sherpa-onnx", "vosk", "silero-vad", "piper", "puter-js"):
    if required not in voice_ids:
        raise SystemExit(f"Voice runtime inventory is missing: {required}")
capability_ids = {item.get("id") for item in ai_catalog.get("capabilities", [])}
for required in ("text-generation", "vision-language", "speech-recognition", "speech-synthesis", "embeddings", "reranking", "ocr-documents", "image-generation", "video-understanding", "agent-tools"):
    if required not in capability_ids:
        raise SystemExit(f"AI capability inventory is missing: {required}")
if not any(item.get("id") == "puter-js" and item.get("status") == "integrated-optional-webview" for item in web_catalog.get("integrations", [])):
    raise SystemExit("Puter.js optional WebView integration catalog entry is missing")
puter_page = ROOT / "android/app/src/main/assets/puter/index.html"
if not puter_page.exists():
    raise SystemExit("Puter.js workspace HTML asset is missing")
puter_html = puter_page.read_text(encoding="utf-8")
for marker in ("https://js.puter.com/v2/", "puter.ai.chat", "puter.ai.txt2speech", "puter.ai.speech2txt", "puter.ai.img2txt", "puter.ai.txt2img"):
    if marker not in puter_html:
        raise SystemExit(f"Puter.js workspace feature is missing: {marker}")
if "addJavascriptInterface" in activity:
    raise SystemExit("Puter WebView must not expose a native JavaScript bridge")
for marker in ("SpeechRecognizer.createOnDeviceSpeechRecognizer", "isOnDeviceRecognitionAvailable", "RECORD_AUDIO", "VOICE_PERMISSION", "stopVoiceRecognition", "private void puterWorkspace", "private void aiCapabilities"):
    if marker not in activity:
        raise SystemExit(f"Voice / Puter UI integration is missing: {marker}")
if '"voice_tts_rate"' not in activity or '"voice_asr_mode"' not in activity:
    raise SystemExit("Voice preferences are missing from settings export")
print("OK Phase 30 voice/ASR, AI capability catalogs, Puter.js WebView and settings")


# Phase 31: universal chat, local chat archive, attachments, voice chat and SEED growth.
integration_catalog = json.loads((ROOT / "config/integration_catalog.json").read_text(encoding="utf-8"))
integration_ids = {item.get("id") for item in integration_catalog.get("integrations", [])}
for required in ("puter-js", "android-saf", "guardian-tools", "seed-skills", "mcp-http", "github", "dropbox", "google-drive", "plugin-sandbox"):
    if required not in integration_ids:
        raise SystemExit(f"Integration catalogue is missing: {required}")
for marker in (
    "private void pickChatAttachments", "ACTION_OPEN_DOCUMENT", "EXTRA_ALLOW_MULTIPLE",
    "private void showChatModelPicker", "private void showChatHistory", "private void startNewChat",
    "private void startVoiceChat", "private void speakAssistantIfEnabled",
    "private void integrations()", "private void growth()", "private void configureMcpEndpoint",
    "PICK_CHAT_ATTACHMENTS", "voice_chat_auto_speak", "chat_archive",
):
    if marker not in activity:
        raise SystemExit(f"Phase 31 chat / growth feature is missing: {marker}")
if "addJavascriptInterface" in activity:
    raise SystemExit("SEED WebViews must not expose an unrestricted native JavaScript bridge")
print("OK Phase 31 universal chat, attachments, model selection, chat archive, voice chat, integrations and growth")
