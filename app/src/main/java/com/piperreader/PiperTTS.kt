package com.piperreader

object PiperTTS {

    // For now this is just a stub.
    // Once we integrate the real libpiper via C++, this will call the JNI method.
    // e.g.: external fun synthesizeText(text: String, outputPath: String): Boolean

    fun synthesizeToFile(text: String, outputPath: String): Boolean {
        // MOCK IMPLEMENTATION FOR UI TEST
        return try {
            val file = java.io.File(outputPath)
            file.writeText("MOCK WAV AUDIO DATA FOR: $text")
            true
        } catch (e: Exception) {
            false
        }
    }
}
