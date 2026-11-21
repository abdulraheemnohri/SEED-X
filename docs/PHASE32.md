# Phase 32 — Build repair and bounded Office attachment extraction

## Build repair
- Removed the duplicate `directoryBytes(File)` method that caused both debug and release Java compilation to fail.
- Added a static regression check requiring exactly one helper declaration.

## Office attachment extraction
- Added `AttachmentTextExtractor` for DOCX, PPTX, XLSX, ODT, ODS and ODP.
- Extraction reads only selected XML text parts from the ZIP container; it does not extract arbitrary files to disk, run macros, or follow external relationships.
- Safety limits: 128 ZIP entries, 256 KiB per selected entry, 2 MiB total expanded selected content, and 64 KiB resulting text.
- Integrated extracted text into the existing untrusted attachment context. Existing plain-text extraction remains capped at 64 KiB.
- Unsupported files remain stored privately and are explicitly described as not inspected.

## Not implemented in this phase
- PDF text extraction, audio transcription, video frame extraction, archive browsing, OCR, and live MCP transport.
- Office extraction is best-effort text only; formatting, charts, images, formulas, comments, embedded objects and some spreadsheet values may be omitted.
- No device-level testing has been performed. GitHub Actions is the build verification source.

## Security boundary
Attachment contents are untrusted user data, not instructions. Extraction is read-only and bounded. It does not execute embedded code or trigger tools. Guardian remains the only authority for tool execution and approval.
