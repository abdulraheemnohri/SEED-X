# Phase 24 — Mobile Navigation and UI Productivity

## Delivered
- Replaced the long horizontal tab strip with a mobile-first four-destination bottom bar: Home, Chat, Models and Settings.
- Added a hamburger/overflow mobile menu with grouped Workspace, AI & Models, Automation & Tools, Security & Health and System destinations.
- Added screen search with live filtering across every registered screen.
- Added per-screen favourites, a favourites menu group, Home quick-launch grid and favourite management in Settings.
- Added configurable startup screen selection, persisted across app restarts.
- Added quick menu actions for new chat, Guardian emergency stop, generation cancellation and optional donations.
- Android Back returns to Home when a secondary screen is open.

## Interaction and safety
- Menu navigation is local UI navigation; it does not grant Android permissions or perform hidden device actions.
- Emergency stop is explicit and uses the existing Guardian native stop boundary; resuming remains a separate Guardian action.
- Generation cancellation requests cancellation and disconnects the active provider connection.
- Favourite and startup preferences are stored in app-private SharedPreferences.
- No new Android permissions were added.

## UI notes
- Existing high-contrast and text scaling preferences apply across the new navigation shell.
- The interface continues to use the dark palette. Full light/system theme switching and drag-and-drop workflow editing remain separate work.
