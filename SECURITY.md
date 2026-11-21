# Security Policy

## Reporting
Please report security issues privately through GitHub's repository security reporting feature when available, or contact the maintainer through the contact information on the GitHub profile. Do not publish exploit details before a fix is available.

## Security design
- System 1 Guardian is the policy and authorization boundary; model output is untrusted.
- No shell, arbitrary command execution, Accessibility automation or privileged device-control API is exposed.
- Public provider endpoints require HTTPS. Local HTTP is opt-in and restricted to loopback/private-LAN endpoints by app validation.
- Provider credentials are encrypted with Android Keystore AES-GCM.
- Diagnostics are local-only and require user review before sharing.
- Model downloads are imported only after GGUF magic-byte validation; this is format validation, not cryptographic provenance verification.

## Supported versions
Security fixes target the current main branch. Users should update to the latest reviewed release.
