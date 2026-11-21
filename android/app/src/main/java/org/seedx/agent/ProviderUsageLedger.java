package org.seedx.agent;

import android.content.Context;
import android.content.SharedPreferences;

/** Privacy-safe local counters only; never stores prompts, responses, keys or provider billing data. */
public final class ProviderUsageLedger {
    private static final String PREFS = "seedx_provider_usage";
    private ProviderUsageLedger() {}

    private static SharedPreferences prefs(Context context) {
        return context.getApplicationContext().getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }

    public static synchronized void recordAttempt(Context context) {
        SharedPreferences p = prefs(context);
        p.edit().putLong("attempts", p.getLong("attempts", 0) + 1).apply();
    }

    public static synchronized void recordSuccess(Context context) {
        SharedPreferences p = prefs(context);
        p.edit().putLong("successes", p.getLong("successes", 0) + 1).apply();
    }

    public static synchronized void recordFailure(Context context) {
        SharedPreferences p = prefs(context);
        p.edit().putLong("failures", p.getLong("failures", 0) + 1).apply();
    }

    public static synchronized String summary(Context context) {
        SharedPreferences p = prefs(context);
        return "Attempts: " + p.getLong("attempts", 0)
            + "  •  Successful: " + p.getLong("successes", 0)
            + "  •  Failed: " + p.getLong("failures", 0)
            + "\nLocal counters only; provider-side tokens, costs and quotas are not available here.";
    }

    public static synchronized void reset(Context context) {
        prefs(context).edit().clear().apply();
    }
}
