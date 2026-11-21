# Phase 12 — Android OS Integration

## Scope

Phase 12 adds an explicit, user-controlled Android foreground service foundation and a permission/status panel. It does not grant SEED-X unrestricted operating-system control.

## Android integration

- Declares Internet and network-state permissions already used by online provider routing.
- Declares foreground-service and foreground-service data-sync permissions for the explicit service.
- Declares Android 13+ notification permission; the app asks at runtime before starting the service.
- Adds an exported=false foreground service with a persistent, low-importance notification and an explicit Stop action.
- Adds Settings links for app permissions and battery-optimization settings.
- Displays notification permission and service state.

## Background execution limits

The service is started only after the user taps Start. It uses the Android `dataSync` foreground-service type and is subject to Android's foreground-service restrictions and time limits (including Android 15 data-sync limits). It is not an always-on daemon and does not restart itself after reboot. For deferred work, use WorkManager in a later phase.

The service is a lifecycle/availability foundation only: it does not execute AI tools, access microphone/location/camera, read notifications, control other apps, or bypass Guardian. Closing the Activity does not stop an explicitly started foreground service; the user can stop it from the persistent notification or the Android service panel.

## Permissions intentionally not requested

No Accessibility, overlay, contacts, SMS, call log, camera, microphone, location, Bluetooth, notification-listener, package-install, all-files-access, device-admin, or VPN permission is requested. Add each only alongside a specific user-facing feature, disclosure, least-privilege design, and Android policy review.

## Build and verification

CI assembles the Android debug APK. Device-level validation is still required for notification permission prompts, foreground-service notification behavior, Android 12–15 background-start restrictions, and OEM battery management.