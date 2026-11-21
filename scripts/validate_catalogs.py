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
