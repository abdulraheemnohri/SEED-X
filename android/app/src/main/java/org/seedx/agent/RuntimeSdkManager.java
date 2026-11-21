package org.seedx.agent;

import android.content.Context;
import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Reports SDK presence without pretending that classpath detection proves
 * model loading, accelerator support, or a working SEED-X inference adapter.
 */
public final class RuntimeSdkManager {
    private RuntimeSdkManager() {}

    private static boolean present(String className) {
        try {
            Class.forName(className, false, RuntimeSdkManager.class.getClassLoader());
            return true;
        } catch (Throwable ignored) {
            return false;
        }
    }

    public static String snapshot(Context context) {
        JSONArray rows = new JSONArray();
        add(rows, "LiteRT-LM", "com.google.ai.edge.litertlm.Engine",
            "LiteRT-LM SDK detected. SEED-X adapter class is compiled; load a compatible .litertlm file to verify runtime readiness.");
        add(rows, "ExecuTorch", "org.pytorch.executorch.extension.llm.LlmModule",
            "ExecuTorch LLM SDK probe. SEED-X reflection adapter is compiled; .pte plus matching tokenizer are required. Upstream LLM API is experimental.");
        add(rows, "ONNX Runtime Mobile", "ai.onnxruntime.OrtEnvironment",
            "ONNX Runtime Android API is visible. This confirms ONNX inference SDK only, not ONNX Runtime GenAI.");
        add(rows, "ONNX Runtime GenAI", "ai.onnxruntime.genai.Model",
            "GenAI Java package availability probe. Official Java API package publication is pending; a native/source build may be required.");
        add(rows, "MLC LLM", "ai.mlc.mlcllm.ChatModule",
            "MLC Android runtime requires its generated mlc4j Gradle subproject and model-specific compiled libraries; not bundled in this generic APK.");
        add(rows, "MNN-LLM", "com.alibaba.mnnllm.android.MnnLlm",
            "MNN Android package probe. Build and package the matching MNN LLM SDK before enabling a SEED-X adapter.");
        return rows.toString();
    }

    private static void add(JSONArray rows, String name, String className, String detail) {
        boolean installed = present(className);
        JSONObject row = new JSONObject();
        try {
            row.put("name", name);
            row.put("sdkPresent", installed);
            row.put("status", installed ? (("LiteRT-LM".equals(name) || "ExecuTorch".equals(name)) ? "ADAPTER_AVAILABLE_model-not-loaded" : "SDK_PRESENT_adapter-pending") : "SDK_NOT_PACKAGED");
            row.put("detail", detail);
            row.put("classProbe", className);
            rows.put(row);
        } catch (Exception ignored) {
            // A diagnostic row must never crash app startup.
        }
    }
}
