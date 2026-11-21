# Phase 26 — Device Status & Backend Management

## Delivered
- Added a dedicated Device Status screen and registered it in the adaptive sidebar, mobile navigation menu, screen search registry and Home quick-launch grid.
- Device identity: manufacturer/model/brand/device, Android release/API, build display, supported ABIs, logical CPU count, display width and density.
- Battery: charge level, charging/discharging/full state, AC/USB/wireless source, health, temperature and voltage from Android's battery status broadcast.
- Power: battery-saver state, whether Android battery optimization currently exempts the app, and thermal status on Android 10+ when exposed by the device.
- Memory/storage: total and available system RAM, Android low-memory and low-RAM indicators, internal data-volume free/total capacity, and app-owned model/download directory sizes.
- Hardware feature reporting for Wi-Fi, Bluetooth, NFC, GPS, camera, microphone hardware, telephony and touchscreen using PackageManager feature declarations.
- Added a Backend Manager screen with active endpoint, configured profile inventory, embedded llama.cpp runtime state, model-loaded state, network status and provider catalog totals.
- Backend Manager exposes explicit user controls for remote/cloud fallback and private-LAN HTTP. Cloud requests still require per-send confirmation; public endpoints require HTTPS.
- Added CI static guards for the Device and Backend screen routes and key status/security controls.

## Permission and privacy boundary
- No new Android permissions were added in this phase.
- Device status uses public Android system APIs and a sticky battery-status broadcast. Hardware feature checks do not activate those features.
- The app does not toggle radios, change system settings, read continuous sensor streams, inspect other apps, or request accessibility, overlay, camera, microphone, location, contacts, SMS, call-log, notification-listener, all-files or device-admin access.
- Storage size is limited to SEED-X app-owned model/download directories. No external storage scan is performed.

## Provider and runtime scope
- Provider catalog entries are connection presets, not guaranteed live services. The catalog currently has 24 profiles; custom OpenAI-compatible endpoints can extend compatibility.
- Provider-specific authentication, model IDs, free tiers, quotas, pricing and regional availability remain controlled by each provider and must be checked at setup time.
- Backend profiles connect to remote/local servers; they do not install those servers on Android.
- llama.cpp is the embedded native inference backend when built with `-PseedxLlamaCpp=ON`. The other runtime catalog entries remain catalog-only until a real SDK adapter, packaging, lifecycle management and device tests are implemented.
- Device telemetry and power/thermal readings can vary by OEM. Physical-device validation remains required.

## Build and test
Run the existing Windows PowerShell build and host-test instructions in the README. CI validates catalog consistency, permission boundaries, Android compilation and native tests. The new device telemetry still requires on-device testing; static CI cannot prove OEM hardware readings.
