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
    "android.permission.RECORD_AUDIO",
    "android.permission.CAMERA",
):
    if forbidden in permissions:
        raise SystemExit(f"High-risk permission requires security review: {forbidden}")
print("OK Android manifest permission boundary")

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
for screen in ("Home", "Chat", "Voice", "Memory", "Prompts", "Models", "Downloads", "Runtimes",
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




