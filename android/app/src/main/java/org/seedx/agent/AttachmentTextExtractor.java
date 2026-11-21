package org.seedx.agent;

import android.content.Context;

import com.tom_roush.pdfbox.android.PDFBoxResourceLoader;
import com.tom_roush.pdfbox.pdmodel.PDDocument;
import com.tom_roush.pdfbox.text.PDFTextStripper;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Bounded, best-effort text extraction for common Office and PDF attachments.
 * Does not execute macros, follow external relationships, or unpack arbitrary files.
 */
public final class AttachmentTextExtractor {
    private static final int MAX_ENTRIES = 128;
    private static final int MAX_ENTRY_BYTES = 256 * 1024;
    private static final int MAX_TOTAL_BYTES = 2 * 1024 * 1024;
    private static final int MAX_OUTPUT_CHARS = 64 * 1024;
    private static final int MAX_PDF_PAGES = 40;

    private AttachmentTextExtractor() {}

    public static String extractText(Context context, File file, String filename) throws IOException {
        if (file == null || !file.isFile()) return "";
        String lower = filename == null ? "" : filename.toLowerCase(Locale.ROOT);
        if (lower.endsWith(".pdf")) return extractPdfText(context, file);
        return extractOfficeText(file, filename);
    }

    private static String extractPdfText(Context context, File file) throws IOException {
        PDFBoxResourceLoader.init(context.getApplicationContext());
        try (PDDocument document = PDDocument.load(file)) {
            if (document.isEncrypted()) throw new IOException("Encrypted PDFs are not extracted.");
            PDFTextStripper stripper = new PDFTextStripper();
            stripper.setStartPage(1);
            stripper.setEndPage(Math.min(document.getNumberOfPages(), MAX_PDF_PAGES));
            stripper.setSortByPosition(true);
            BoundedWriter output = new BoundedWriter(MAX_OUTPUT_CHARS);
            stripper.writeText(document, output);
            return output.toString().trim();
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            throw new IOException("Could not safely extract PDF text: " + e.getMessage(), e);
        }
    }

    private static final class BoundedWriter extends Writer {
        private final StringBuilder value = new StringBuilder();
        private final int limit;
        BoundedWriter(int limit) { this.limit = limit; }
        @Override public void write(char[] chars, int offset, int length) {
            int remaining = limit - value.length();
            if (remaining > 0) value.append(chars, offset, Math.min(length, remaining));
        }
        @Override public void flush() {}
        @Override public void close() {}
        @Override public String toString() { return value.toString(); }
    }

    public static String extractOfficeText(File file, String filename) throws IOException {
        if (file == null || !file.isFile()) return "";
        String lower = filename == null ? "" : filename.toLowerCase(Locale.ROOT);
        if (!(lower.endsWith(".docx") || lower.endsWith(".pptx") || lower.endsWith(".xlsx")
                || lower.endsWith(".odt") || lower.endsWith(".ods") || lower.endsWith(".odp"))) return "";

        StringBuilder output = new StringBuilder();
        int entries = 0;
        int totalBytes = 0;
        try (ZipInputStream zip = new ZipInputStream(new FileInputStream(file))) {
            ZipEntry entry;
            byte[] buffer = new byte[8192];
            while ((entry = zip.getNextEntry()) != null) {
                if (++entries > MAX_ENTRIES) throw new IOException("Office document contains too many ZIP entries.");
                String name = entry.getName();
                if (!isTextXmlPart(lower, name) || entry.isDirectory()) {
                    zip.closeEntry();
                    continue;
                }
                ByteArrayOutputStream bytes = new ByteArrayOutputStream();
                int read;
                while ((read = zip.read(buffer)) != -1) {
                    totalBytes += read;
                    if (bytes.size() + read > MAX_ENTRY_BYTES || totalBytes > MAX_TOTAL_BYTES)
                        throw new IOException("Office document expanded content exceeds the safe extraction limit.");
                    bytes.write(buffer, 0, read);
                }
                appendXmlText(output, new String(bytes.toByteArray(), StandardCharsets.UTF_8));
                zip.closeEntry();
                if (output.length() >= MAX_OUTPUT_CHARS) break;
            }
        } catch (java.util.zip.ZipException invalid) {
            throw new IOException("Office attachment is not a valid ZIP-based document.", invalid);
        }
        if (output.length() > MAX_OUTPUT_CHARS) output.setLength(MAX_OUTPUT_CHARS);
        return output.toString().trim();
    }

    private static boolean isTextXmlPart(String ext, String name) {
        if (ext.endsWith(".docx")) return "word/document.xml".equals(name);
        if (ext.endsWith(".pptx")) return name.matches("ppt/slides/slide[0-9]+\\.xml");
        if (ext.endsWith(".xlsx")) return "xl/sharedStrings.xml".equals(name) || name.matches("xl/worksheets/sheet[0-9]+\\.xml");
        return "content.xml".equals(name);
    }

    private static void appendXmlText(StringBuilder out, String xml) {
        boolean inTag = false;
        StringBuilder token = new StringBuilder();
        for (int i = 0; i < xml.length() && out.length() < MAX_OUTPUT_CHARS; i++) {
            char c = xml.charAt(i);
            if (c == '<') {
                inTag = true;
                if (token.length() > 0) appendDecoded(out, token);
                token.setLength(0);
            } else if (c == '>') {
                inTag = false;
            } else if (!inTag) token.append(c);
        }
        if (token.length() > 0) appendDecoded(out, token);
        if (out.length() < MAX_OUTPUT_CHARS) out.append('\n');
    }

    private static void appendDecoded(StringBuilder out, StringBuilder raw) {
        String value = raw.toString().replace("&amp;", "&").replace("&lt;", "<")
                .replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
        value = value.replaceAll("[\\t\\r\\n ]+", " ").trim();
        if (!value.isEmpty() && out.length() < MAX_OUTPUT_CHARS) {
            if (out.length() > 0 && out.charAt(out.length() - 1) != '\\n') out.append(' ');
            int remaining = MAX_OUTPUT_CHARS - out.length();
            out.append(value, 0, Math.min(value.length(), remaining));
        }
    }
}
