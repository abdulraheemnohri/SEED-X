package org.seedx.agent;

import android.app.ActivityManager;
import android.content.Context;
import android.os.Debug;
import java.io.File;

/** Selects one compatible local runtime and enforces a conservative memory admission gate. */
public final class RuntimeAdapterManager {
    private final Context context;
    private InferenceAdapter active;

    public RuntimeAdapterManager(Context context) { this.context = context.getApplicationContext(); }

    public synchronized String load(File model, File tokenizer) throws Exception {
        if (model == null || !model.isFile() || !model.canRead())
            throw new IllegalArgumentException("Select a readable local model file first.");
        String name = model.getName().toLowerCase(java.util.Locale.ROOT);
        InferenceAdapter next;
        if (name.endsWith(".litertlm")) {
            next = new LiteRtLmAdapter(context);
        } else if (name.endsWith(".pte")) {
            next = new ExecuTorchAdapter();
            if (tokenizer == null) throw new IllegalArgumentException("ExecuTorch .pte models require their matching tokenizer file.");
        } else {
            throw new IllegalArgumentException("No Phase 29 adapter for this format. Supported here: .litertlm and .pte. GGUF continues through the existing llama.cpp native path.");
        }
        requireMemoryHeadroom(model);
        if (active != null) { active.unload(); active = null; }
        next.load(model, tokenizer);
        active = next;
        return "Loaded " + next.id() + " model: " + model.getName();
    }

    private void requireMemoryHeadroom(File model) {
        ActivityManager manager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        if (manager == null) return;
        ActivityManager.MemoryInfo info = new ActivityManager.MemoryInfo();
        manager.getMemoryInfo(info);
        long estimated = (long) (model.length() * 1.20d) + 384L * 1024L * 1024L;
        long available = Math.min(info.availMem, Runtime.getRuntime().maxMemory());
        if (available < estimated) {
            throw new IllegalStateException("Not enough available memory for safe loading. Estimated need: "
                + (estimated / 1048576L) + " MB; current available app/system headroom: "
                + (available / 1048576L) + " MB. Close other apps or choose a smaller model.");
        }
    }

    public synchronized void generate(String prompt, int maxTokens, float temperature, InferenceAdapter.Listener listener) {
        if (active == null || !active.isLoaded()) { listener.onError("No local adapter/model is loaded."); return; }
        active.generate(prompt, maxTokens, temperature, listener);
    }
    public synchronized void cancel() { if (active != null) active.cancel(); }
    public synchronized void unload() { if (active != null) { active.unload(); active = null; } }
    public synchronized String activeRuntime() { return active == null ? "none" : active.id(); }
    public synchronized boolean isLoaded() { return active != null && active.isLoaded(); }
}
