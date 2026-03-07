#include <jni.h>
#include <string>
#include <android/log.h>
#include <fstream>
#include <optional>

#define LOG_TAG "PiperJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifdef USE_REAL_PIPER
#include "piper/piper.hpp"
#endif

// Global instance to hold the Piper session
#ifdef USE_REAL_PIPER
static std::optional<piper::PiperConfig> g_piperConfig;
static std::optional<piper::Voice> g_voice;
static bool g_initialized = false;
#endif

extern "C" JNIEXPORT jboolean JNICALL
Java_com_piperreader_PiperTTS_initPiperNative(
        JNIEnv* env,
        jclass clazz,
        jstring jModelPath,
        jstring jConfigPath) {

    const char* modelPathCStr = env->GetStringUTFChars(jModelPath, nullptr);
    const char* configPathCStr = env->GetStringUTFChars(jConfigPath, nullptr);

    std::string modelPath(modelPathCStr);
    std::string configPath(configPathCStr);

    env->ReleaseStringUTFChars(jModelPath, modelPathCStr);
    env->ReleaseStringUTFChars(jConfigPath, configPathCStr);

#ifdef USE_REAL_PIPER
    try {
        if (!g_initialized) {
            g_piperConfig.emplace();
            // Need to set eSpeakDataPath if espeak-ng is used
            g_piperConfig->useESpeak = true;
            piper::initialize(*g_piperConfig);
            g_initialized = true;
        }

        g_voice.emplace();
        std::optional<piper::SpeakerId> speakerId;
        piper::loadVoice(*g_piperConfig, modelPath, configPath, *g_voice, speakerId, false);
        LOGI("Successfully loaded voice from %s", modelPath.c_str());
        return JNI_TRUE;
    } catch (const std::exception& e) {
        LOGE("Failed to load Piper model: %s", e.what());
        return JNI_FALSE;
    }
#else
    LOGI("MOCK: Loading Piper model from: %s", modelPath.c_str());
    return JNI_TRUE;
#endif
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_piperreader_PiperTTS_synthesizeToFileNative(
        JNIEnv* env,
        jclass clazz,
        jstring jText,
        jstring jOutputPath,
        jfloat lengthScale,
        jfloat noiseScale,
        jfloat noiseW,
        jfloat sentenceSilence) {

    const char* textCStr = env->GetStringUTFChars(jText, nullptr);
    const char* outputPathCStr = env->GetStringUTFChars(jOutputPath, nullptr);

    std::string text(textCStr);
    std::string outputPath(outputPathCStr);

    env->ReleaseStringUTFChars(jText, textCStr);
    env->ReleaseStringUTFChars(jOutputPath, outputPathCStr);

#ifdef USE_REAL_PIPER
    if (!g_voice || !g_piperConfig) {
        LOGE("Piper model not loaded!");
        return JNI_FALSE;
    }

    try {
        g_voice->synthesisConfig.lengthScale = lengthScale;
        g_voice->synthesisConfig.noiseScale = noiseScale;
        g_voice->synthesisConfig.noiseW = noiseW;
        g_voice->synthesisConfig.sentenceSilenceSeconds = sentenceSilence;

        std::ofstream audioFile(outputPath, std::ios::binary);
        if (!audioFile.is_open()) {
            LOGE("Failed to open output file: %s", outputPath.c_str());
            return JNI_FALSE;
        }

        piper::SynthesisResult result;
        piper::textToWavFile(*g_piperConfig, *g_voice, text, audioFile, result);

        LOGI("Synthesis complete: infer=%.2fs, audio=%.2fs, rtf=%.2f",
             result.inferSeconds, result.audioSeconds, result.realTimeFactor);

        return JNI_TRUE;
    } catch (const std::exception& e) {
        LOGE("Synthesis failed: %s", e.what());
        return JNI_FALSE;
    }
#else
    LOGI("Synthesizing text: '%s'", text.c_str());
    LOGI("Output: %s", outputPath.c_str());
    LOGI("Config: speed=%.2f, noise=%.2f, noiseW=%.2f, silence=%.2f",
            lengthScale, noiseScale, noiseW, sentenceSilence);

    FILE* f = fopen(outputPath.c_str(), "w");
    if (f) {
        fprintf(f, "NATIVE C++ MOCK WAV AUDIO DATA FOR: %s\n", text.c_str());
        fprintf(f, "Settings:\nLength Scale: %.3f\nNoise Scale: %.3f\nNoise W: %.3f\nSentence Silence: %.3f\n",
                lengthScale, noiseScale, noiseW, sentenceSilence);
        fclose(f);
        return JNI_TRUE;
    } else {
        LOGE("Failed to write to %s", outputPath.c_str());
        return JNI_FALSE;
    }
#endif
}
