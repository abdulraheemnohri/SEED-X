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

/** Local-only crash recorder. Reports are never transmitted automatically. */
public final class CrashReporter {
    private static final String DIRECTORY = "crash-reports";
    private static final String FILE_NAME = "latest-crash.txt";
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
        String report = "SEED-X Diagnostic Report\n"
            + "Type: " + kind + "\n"
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
        File file = latest(context);
        if (file != null) file.delete();
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
