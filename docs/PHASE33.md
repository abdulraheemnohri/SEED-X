# Phase 33 — Bounded PDF text extraction

- Adds PdfBox-Android 2.0.27.0 from Maven Central.
- PDF attachments are parsed locally after being copied into app-private storage.
- Text extraction reads at most the first 40 pages and writes at most 64 KiB of output.
- Encrypted PDFs are rejected; malformed or unsupported PDFs report an import failure instead of being represented as inspected content.
- PDF extraction is integrated into the existing attachment prompt. Extracted content remains untrusted user data.
- Existing 20 MiB per-file attachment cap still applies.

## Limitations
- Scanned/image-only PDFs do not become searchable; OCR is not included.
- Very complex or malformed PDFs can still consume parser resources before output truncation. Extraction runs on the worker executor, never the UI thread.
- No PDF parser security audit or physical-device performance test is claimed. The dependency and Android build must pass CI before release use.
