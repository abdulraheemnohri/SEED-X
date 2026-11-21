package org.seedx.agent;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * ExecuTorch LLM adapter. Reflection keeps the app's lightweight/stub builds
 * compilable when the optional ExecuTorch AAR is not selected. At runtime this
 * adapter calls the upstream experimental LlmModule API from executorch-android.
 */
public final class ExecuTorchAdapter implements InferenceAdapter {
    private volatile Object module;
    private volatile Class<?> callbackType;
    private final AtomicBoolean cancelled = new AtomicBoolean(false);

    @Override public String id() { return "executorch"; }

    @Override public synchronized void load(File modelFile, File tokenizerFile) throws Exception {
        unload();
        if (modelFile == null || !modelFile.isFile() || !modelFile.canRead())
            throw new IllegalArgumentException("ExecuTorch model .pte file is missing or unreadable.");
        if (tokenizerFile == null || !tokenizerFile.isFile() || !tokenizerFile.canRead())
            throw new IllegalArgumentException("ExecuTorch tokenizer file is required and must be readable.");
        ClassLoader loader = getClass().getClassLoader();
        Class<?> moduleType = Class.forName("org.pytorch.executorch.extension.llm.LlmModule", true, loader);
        callbackType = Class.forName("org.pytorch.executorch.extension.llm.LlmCallback", true, loader);
        Constructor<?> constructor = moduleType.getConstructor(String.class, String.class, float.class);
        Object candidate = constructor.newInstance(modelFile.getAbsolutePath(), tokenizerFile.getAbsolutePath(), 0.7f);
        try {
            Object result = moduleType.getMethod("load").invoke(candidate);
            int status = result instanceof Number ? ((Number) result).intValue() : -1;
            if (status != 0) throw new IllegalStateException("ExecuTorch model load failed with status " + status);
            module = candidate;
        } catch (Exception failure) {
            releaseObject(candidate);
            throw failure;
        }
    }

    @Override public void generate(String prompt, int maxTokens, float temperature, Listener listener) {
        final Object active = module;
        if (active == null) { listener.onError("ExecuTorch model is not loaded."); return; }
        if (prompt == null || prompt.trim().isEmpty()) { listener.onError("Prompt is empty."); return; }
        cancelled.set(false);
        final java.util.concurrent.atomic.AtomicBoolean failed = new java.util.concurrent.atomic.AtomicBoolean(false);
        try {
            ClassLoader loader = getClass().getClassLoader();
            Class<?> configType = Class.forName("org.pytorch.executorch.extension.llm.LlmGenerationConfig", true, loader);
            Object builder = configType.getMethod("create").invoke(null);
            invokeIfPresent(builder, "temperature", new Class<?>[]{float.class}, new Object[]{temperature});
            invokeIfPresent(builder, "maxNewTokens", new Class<?>[]{int.class}, new Object[]{Math.max(1, Math.min(maxTokens, 4096))});
            invokeIfPresent(builder, "echo", new Class<?>[]{boolean.class}, new Object[]{false});
            Object config = builder.getClass().getMethod("build").invoke(builder);
            Object callback = Proxy.newProxyInstance(loader, new Class<?>[]{callbackType}, (proxy, method, args) -> {
                String name = method.getName();
                if ("onResult".equals(name) && args != null && args.length > 0 && !cancelled.get()) {
                    listener.onToken(String.valueOf(args[0]));
                } else if ("onError".equals(name)) {
                    String message = args != null && args.length > 1 ? String.valueOf(args[1]) : "ExecuTorch generation failed.";
                    failed.set(true); listener.onError(message);
                }
                return null;
            });
            active.getClass().getMethod("generate", String.class, configType, callbackType)
                .invoke(active, prompt, config, callback);
            if (!cancelled.get() && !failed.get()) listener.onComplete();
        } catch (Throwable error) {
            Throwable cause = error.getCause() == null ? error : error.getCause();
            listener.onError("ExecuTorch: " + (cause.getMessage() == null ? cause.getClass().getSimpleName() : cause.getMessage()));
        }
    }

    private static void invokeIfPresent(Object target, String name, Class<?>[] types, Object[] args) {
        try { target.getClass().getMethod(name, types).invoke(target, args); }
        catch (ReflectiveOperationException ignored) { /* older SDK: retain upstream defaults */ }
    }

    @Override public void cancel() {
        cancelled.set(true);
        Object active = module;
        if (active != null) {
            try { active.getClass().getMethod("stop").invoke(active); }
            catch (ReflectiveOperationException ignored) { }
        }
    }

    @Override public synchronized void unload() {
        cancel();
        Object old = module;
        module = null;
        callbackType = null;
        releaseObject(old);
    }

    private static void releaseObject(Object value) {
        if (value == null) return;
        // API revisions differ; use an explicit native release method when present.
        for (String method : new String[]{"close", "destroy", "release"}) {
            try { value.getClass().getMethod(method).invoke(value); return; }
            catch (ReflectiveOperationException ignored) { }
        }
    }

    @Override public boolean isLoaded() { return module != null; }
}
