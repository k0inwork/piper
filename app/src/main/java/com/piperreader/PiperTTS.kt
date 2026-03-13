package com.piperreader

import android.util.Log

object PiperTTS {

    var initError: Throwable? = null

    init {
        try {
            System.loadLibrary("onnxruntime")
            System.loadLibrary("piper_android")
            System.loadLibrary("piperreader")
        } catch (e: UnsatisfiedLinkError) {
            initError = e
            Log.e("PiperTTS", "Native library not found, falling back to mock implementation.", e)
        }
    }

    // JNI Declarations
    @JvmStatic
    external fun initPiperNative(modelPath: String, configPath: String): Boolean

    @JvmStatic
    external fun synthesizeToFileNative(
        text: String,
        outputPath: String,
        lengthScale: Float,
        noiseScale: Float,
        noiseW: Float,
        sentenceSilence: Float
    ): Boolean

    // Wrapper functions that fall back to mock if JNI fails
    fun initPiper(modelPath: String, configPath: String): Boolean {
        return try {
            initPiperNative(modelPath, configPath)
        } catch (e: UnsatisfiedLinkError) {
            Log.w("PiperTTS", "Mock initPiper called with model=$modelPath, config=$configPath")
            true
        }
    }

    fun synthesizeToFile(
        text: String,
        outputPath: String,
        lengthScale: Float = 1.0f,
        noiseScale: Float = 0.667f,
        noiseW: Float = 0.8f,
        sentenceSilence: Float = 0.2f
    ): Boolean {
        return try {
            // Attempt to call the native C++ implementation
            synthesizeToFileNative(text, outputPath, lengthScale, noiseScale, noiseW, sentenceSilence)
        } catch (e: UnsatisfiedLinkError) {
            // MOCK IMPLEMENTATION FOR UI TEST / MISSING NATIVE LIB
            Log.w("PiperTTS", "Calling mock synthesizeToFile because native method is missing.", e)
            try {
                val file = java.io.File(outputPath)
                val errorToLog = initError ?: e
                val mockContent = "MOCK_ERROR: " + errorToLog.message + "\n" + Log.getStackTraceString(errorToLog)
                file.writeText(mockContent)
                true
            } catch (ex: Exception) {
                false
            }
        }
    }
}
