#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>
#include <fstream>
#include <stdexcept>

#define LOG_TAG "PiperJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifdef USE_REAL_PIPER
#include "piper.h"

static piper_synthesizer* g_piper_synth = nullptr;
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
    LOGI("Loading Piper model from %s", modelPath.c_str());

    if (g_piper_synth) {
        piper_free(g_piper_synth);
        g_piper_synth = nullptr;
    }

    g_piper_synth = piper_create(modelPath.c_str(), configPath.c_str(), nullptr);
    if (!g_piper_synth) {
        LOGE("Failed to initialize Piper model");
        return JNI_FALSE;
    }

    LOGI("Successfully loaded Piper model");
    return JNI_TRUE;
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
    if (!g_piper_synth) {
        LOGE("Piper model not loaded!");
        return JNI_FALSE;
    }

    LOGI("Synthesizing text to %s", outputPath.c_str());

    piper_synthesize_options options = piper_default_synthesize_options(g_piper_synth);
    options.length_scale = lengthScale;
    options.noise_scale = noiseScale;
    options.noise_w_scale = noiseW;

    if (piper_synthesize_start(g_piper_synth, text.c_str(), &options) != PIPER_OK) {
        LOGE("piper_synthesize_start failed!");
        return JNI_FALSE;
    }

    std::vector<int16_t> all_samples;
    int sample_rate = 22050; // Fallback

    piper_audio_chunk chunk;
    int res;
    while ((res = piper_synthesize_next(g_piper_synth, &chunk)) == PIPER_OK) {
        sample_rate = chunk.sample_rate;
        for (size_t i = 0; i < chunk.num_samples; ++i) {
            float s = chunk.samples[i];
            if (s > 1.0f) s = 1.0f;
            if (s < -1.0f) s = -1.0f;
            all_samples.push_back(static_cast<int16_t>(s * 32767.0f));
        }
        if (chunk.is_last) break;
    }

    if (res != PIPER_OK && res != PIPER_DONE) {
         LOGE("piper_synthesize_next failed with %d", res);
         return JNI_FALSE;
    }

    // Write to WAV
    std::ofstream out_file(outputPath, std::ios::binary);
    if (!out_file) {
        LOGE("Failed to open output file: %s", outputPath.c_str());
        return JNI_FALSE;
    }

    // Write WAV header
    auto writeWavHeader = [](std::ofstream& file, int sampleRate, size_t numSamples) {
        uint32_t byteRate = sampleRate * sizeof(int16_t);
        uint32_t dataSize = numSamples * sizeof(int16_t);
        uint32_t chunkSize = 36 + dataSize;

        file.write("RIFF", 4);
        file.write(reinterpret_cast<const char*>(&chunkSize), 4);
        file.write("WAVE", 4);
        file.write("fmt ", 4);

        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 1;
        uint16_t numChannels = 1;
        uint16_t blockAlign = sizeof(int16_t);
        uint16_t bitsPerSample = 16;

        file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
        file.write(reinterpret_cast<const char*>(&audioFormat), 2);
        file.write(reinterpret_cast<const char*>(&numChannels), 2);
        file.write(reinterpret_cast<const char*>(&sampleRate), 4);
        file.write(reinterpret_cast<const char*>(&byteRate), 4);
        file.write(reinterpret_cast<const char*>(&blockAlign), 2);
        file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

        file.write("data", 4);
        file.write(reinterpret_cast<const char*>(&dataSize), 4);
    };

    writeWavHeader(out_file, sample_rate, all_samples.size());
    out_file.write(reinterpret_cast<const char*>(all_samples.data()), all_samples.size() * sizeof(int16_t));
    out_file.close();

    LOGI("Synthesis complete");
    return JNI_TRUE;
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
