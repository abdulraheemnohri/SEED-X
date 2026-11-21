package org.seedx.agent;

import java.io.File;

/** Shared contract for real, model-format-specific local inference engines. */
public interface InferenceAdapter {
    interface Listener {
        void onToken(String token);
        void onComplete();
        void onError(String message);
    }
    String id();
    void load(File modelFile, File tokenizerFile) throws Exception;
    void generate(String prompt, int maxTokens, float temperature, Listener listener);
    void cancel();
    void unload();
    boolean isLoaded();
}
