package org.seedx.agent;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Arrays;
import java.security.MessageDigest;

/** Local-only crash recorder. Reports are never transmitted automatically. */
public final class CrashReporter {
    private static final String DIRECTORY = "crash-reports";
    private static final String FILE_NAME = "latest-crash.txt";
    private static final String BREADCRUMB_FILE = "breadcrumbs.txt";
    private static final int MAX_HISTORY = 10;
    private static final int MAX_BYTES = 96 * 1024;
    private static volatile Thread.UncaughtExceptionHandler previous;

    private CrashReporter() {}

    public static void install(Context context) {
        final Context app = context.getApplicationContext();
        if (previous != null) return;
        previous = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler((thread, error) -> {
            try { record(app, "FATAL CRASH", thread, error); } catch (Throwable ignored) {}
            Thread.UncaughtExceptionHandler delegate = previous;
            if (delegate != null) delegate.uncaughtException(thread, error);
        });
    }

    public static synchronized void recordNonFatal(Context context, String area, Throwable error) {
        try { record(context.getApplicationContext(), "NON-FATAL ERROR: " + safe(area), Thread.currentThread(), error); }
        catch (Throwable ignored) {}
    }

    private static synchronized void record(Context context, String kind, Thread thread, Throwable error) throws Exception {
        File dir = new File(context.getFilesDir(), DIRECTORY);
        if (!dir.exists() && !dir.mkdirs()) return;
        File target = new File(dir, FILE_NAME);
        StringWriter stack = new StringWriter();
        if (error != null) error.printStackTrace(new PrintWriter(stack));
        String stackText = stack.toString();
        String report = "SEED-X Diagnostic Report\n"
            + "Type: " + kind + "\n"
            + "Category: " + classify(kind + " " + stackText) + "\n"
            + "Fingerprint: " + fingerprint(stackText) + "\n"
            + "Time: " + new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssXXX", Locale.US).format(new Date()) + "\n"
            + "App: " + appVersion(context) + "\n"
            + "Android: " + Build.VERSION.RELEASE + " (API " + Build.VERSION.SDK_INT + ")\n"
            + "Device: " + safe(Build.MANUFACTURER) + " " + safe(Build.MODEL) + "\n"
            + "Thread: " + (thread == null ? "unknown" : safe(thread.getName())) + "\n\n"
            + stack;
        byte[] bytes = report.getBytes(StandardCharsets.UTF_8);
        if (bytes.length > MAX_BYTES) {
            report = report.substring(0, Math.min(report.length(), MAX_BYTES / 2)) + "\n[Report truncated for safety]\n";
            bytes = report.getBytes(StandardCharsets.UTF_8);
        }
        try (FileOutputStream out = new FileOutputStream(target, false)) { out.write(bytes); out.getFD().sync(); }
        String stamp = new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.US).format(new Date());
        File history = new File(dir, stamp + "-" + System.currentTimeMillis() + ".txt");
        try (FileOutputStream out = new FileOutputStream(history, false)) { out.write(bytes); out.getFD().sync(); }
        pruneHistory(dir);
    }


    public static String classify(String input) {
        String value = input == null ? "" : input.toLowerCase(Locale.US);
        if (value.contains("outofmemoryerror") || value.contains("failed to allocate")) return "MEMORY";
        if (value.contains("securityexception") || value.contains("permission denied")) return "PERMISSION_OR_SECURITY";
        if (value.contains("sockettimeoutexception") || value.contains("unknownhostexception") || value.contains("connectexception")) return "NETWORK";
        if (value.contains("jsonexception") || value.contains("parse")) return "DATA_OR_PARSING";
        if (value.contains("unsatisfiedlinkerror") || value.contains("jni") || value.contains("native")) return "NATIVE_BRIDGE";
        if (value.contains("illegalstateexception") || value.contains("nullpointerexception") || value.contains("indexoutofboundsexception")) return "APPLICATION_LOGIC";
        return "UNCLASSIFIED";
    }

    private static String fingerprint(String stack) {
        try {
            StringBuilder normalized = new StringBuilder();
            String[] lines = stack.split("\\n");
            int count = 0;
            for (String line : lines) {
                String trimmed = line.trim();
                if (trimmed.startsWith("at ") || trimmed.endsWith("Exception") || trimmed.endsWith("Error")) {
                    normalized.append(trimmed.replaceAll(":\\d+\\)", ":#)")).append('\n');
                    if (++count >= 10) break;
                }
            }
            byte[] hash = MessageDigest.getInstance("SHA-256").digest(normalized.toString().getBytes(StandardCharsets.UTF_8));
            StringBuilder hex = new StringBuilder();
            for (int i = 0; i < 8; i++) hex.append(String.format(Locale.US, "%02x", hash[i]));
            return hex.toString();
        } catch (Exception e) { return "unavailable"; }
    }

    public static synchronized void breadcrumb(Context context, String event) {
        try {
            File dir = new File(context.getFilesDir(), DIRECTORY);
            if (!dir.exists() && !dir.mkdirs()) return;
            File file = new File(dir, BREADCRUMB_FILE);
            String line = new SimpleDateFormat("HH:mm:ss", Locale.US).format(new Date()) + " | " + safe(event) + "\n";
            byte[] old = file.isFile() ? readBytes(file, 12 * 1024) : new byte[0];
            byte[] add = line.getBytes(StandardCharsets.UTF_8);
            byte[] combined = new byte[Math.min(16 * 1024, old.length + add.length)];
            int keep = Math.min(old.length, combined.length - Math.min(add.length, combined.length));
            System.arraycopy(old, Math.max(0, old.length - keep), combined, 0, keep);
            int addCount = Math.min(add.length, combined.length - keep);
            System.arraycopy(add, 0, combined, keep, addCount);
            try (FileOutputStream out = new FileOutputStream(file, false)) { out.write(combined); }
        } catch (Throwable ignored) {}
    }

    public static File[] history(Context context) {
        File dir = new File(context.getFilesDir(), DIRECTORY);
        File[] files = dir.listFiles((d, name) -> name.endsWith(".txt") && !name.equals(FILE_NAME) && !name.equals(BREADCRUMB_FILE));
        if (files == null) return new File[0];
        Arrays.sort(files, (x, y) -> Long.compare(y.lastModified(), x.lastModified()));
        return files;
    }

    public static String readFile(File file) {
        if (file == null || !file.isFile()) return "";
        try { return new String(readBytes(file, MAX_BYTES), StandardCharsets.UTF_8); }
        catch (Exception e) { return "Could not read report."; }
    }

    public static String readBreadcrumbs(Context context) {
        File file = new File(new File(context.getFilesDir(), DIRECTORY), BREADCRUMB_FILE);
        return file.isFile() ? readBounded(file, 16 * 1024) : "No recent app events recorded.";
    }

    private static String readBounded(File file, int max) {
        try { return new String(readBytes(file, max), StandardCharsets.UTF_8); }
        catch (Exception e) { return "Could not read event history."; }
    }

    private static byte[] readBytes(File file, int max) throws Exception {
        try (java.io.FileInputStream in = new java.io.FileInputStream(file)) {
            byte[] data = new byte[(int)Math.min(file.length(), max)];
            int count = in.read(data);
            return count <= 0 ? new byte[0] : Arrays.copyOf(data, count);
        }
    }

    private static void pruneHistory(File dir) {
        File[] files = dir.listFiles((d, name) -> name.endsWith(".txt") && !name.equals(FILE_NAME) && !name.equals(BREADCRUMB_FILE));
        if (files == null || files.length <= MAX_HISTORY) return;
        Arrays.sort(files, (x, y) -> Long.compare(y.lastModified(), x.lastModified()));
        for (int i = MAX_HISTORY; i < files.length; i++) files[i].delete();
    }

    public static File latest(Context context) {
        File file = new File(new File(context.getFilesDir(), DIRECTORY), FILE_NAME);
        return file.isFile() ? file : null;
    }

    public static String readLatest(Context context) {
        File file = latest(context);
        if (file == null) return "";
        try {
            byte[] data = new byte[(int)Math.min(file.length(), MAX_BYTES)];
            try (java.io.FileInputStream in = new java.io.FileInputStream(file)) {
                int count = in.read(data);
                return count <= 0 ? "" : new String(data, 0, count, StandardCharsets.UTF_8);
            }
        } catch (Exception e) { return "Could not read diagnostic report: " + e.getMessage(); }
    }

    public static void clear(Context context) {
        File dir = new File(context.getFilesDir(), DIRECTORY);
        File[] files = dir.listFiles();
        if (files != null) for (File file : files) if (file.isFile()) file.delete();
    }

    private static String appVersion(Context context) {
        try {
            PackageInfo info = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
            return info.versionName + " (" + info.getLongVersionCode() + ")";
        } catch (PackageManager.NameNotFoundException e) { return "unknown"; }
    }

    private static String safe(String value) {
        if (value == null) return "unknown";
        return value.replaceAll("[\\r\\n\\t]", " ").replaceAll("[^\\p{Print}]", "?");
    }
}
