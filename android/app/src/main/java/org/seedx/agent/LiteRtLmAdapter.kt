package org.seedx.agent

import android.content.Context
import com.google.ai.edge.litertlm.Backend
import com.google.ai.edge.litertlm.Conversation
import com.google.ai.edge.litertlm.Engine
import com.google.ai.edge.litertlm.EngineConfig
import java.io.File
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicBoolean
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.collect

/**
 * Real LiteRT-LM conversation adapter. A single-thread coroutine dispatcher
 * owns Engine/Conversation creation, streaming, cancellation and release.
 * Model weights stay in app-private files and are never packaged in the APK.
 */
class LiteRtLmAdapter(context: Context) : InferenceAdapter {
    private val appContext = context.applicationContext
    private val executor = Executors.newSingleThreadExecutor()
    private val dispatcher = executor.asCoroutineDispatcher()
    private val scope = CoroutineScope(SupervisorJob() + dispatcher)
    private val cancelled = AtomicBoolean(false)
    @Volatile private var engine: Engine? = null
    @Volatile private var conversation: Conversation? = null
    @Volatile private var generationJob: Job? = null

    override fun id(): String = "litert-lm"

    override fun load(modelFile: File, tokenizerFile: File?) {
        require(modelFile.isFile && modelFile.canRead() && modelFile.name.endsWith(".litertlm", true)) {
            "LiteRT-LM requires a readable .litertlm model package."
        }
        val future = executor.submit {
            releaseOwned()
            val candidate = Engine(
                EngineConfig(
                    modelPath = modelFile.absolutePath,
                    backend = Backend.CPU(),
                    cacheDir = appContext.cacheDir.absolutePath
                )
            )
            try {
                candidate.initialize()
                engine = candidate
                conversation = candidate.createConversation()
            } catch (failure: Throwable) {
                // Do not close an Engine whose initialize() failed.
                throw failure
            }
        }
        future.get()
    }

    override fun generate(prompt: String, maxTokens: Int, temperature: Float, listener: InferenceAdapter.Listener) {
        val current = conversation
        if (current == null) {
            listener.onError("LiteRT-LM model is not loaded.")
            return
        }
        if (prompt.isBlank()) {
            listener.onError("Prompt is empty.")
            return
        }
        cancelled.set(false)
        generationJob = scope.launch {
            try {
                current.sendMessageAsync(prompt, maxOutputToken = maxTokens.coerceIn(1, 4096)).collect { chunk ->
                    if (!cancelled.get()) listener.onToken(chunk.toString())
                }
                if (!cancelled.get()) listener.onComplete()
            } catch (failure: Throwable) {
                if (!cancelled.get()) listener.onError("LiteRT-LM: ${failure.message ?: failure.javaClass.simpleName}")
            } finally { generationJob = null }
        }
    }

    override fun cancel() {
        cancelled.set(true)
        scope.launch { conversation?.cancelProcess() }
    }

    override fun unload() {
        val finished = java.util.concurrent.CompletableFuture<Unit>()
        scope.launch {
            try {
                conversation?.cancelProcess()
                generationJob?.join()
                releaseOwned()
                finished.complete(Unit)
            } catch (failure: Throwable) { finished.completeExceptionally(failure) }
        }
        finished.get()
    }

    private fun releaseOwned() {
        cancelled.set(true)
        val oldConversation = conversation
        conversation = null
        try { oldConversation?.cancelProcess() } catch (_: Throwable) { }
        try { oldConversation?.close() } catch (_: Throwable) { }
        val oldEngine = engine
        engine = null
        try { oldEngine?.close() } catch (_: Throwable) { }
    }

    override fun isLoaded(): Boolean = engine != null && conversation != null

    fun shutdown() {
        unload()
        scope.cancel()
        dispatcher.close()
        executor.shutdown()
    }
}
