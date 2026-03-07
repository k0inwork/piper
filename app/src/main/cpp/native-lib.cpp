#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "PiperJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Hypothetical structs that represent the real libpiper C++ API
namespace piper {
    struct SynthesisConfig {
        float lengthScale = 1.0f;
        float noiseScale = 0.667f;
        float noiseW = 0.8f;
        float sentenceSilenceSeconds = 0.2f;
    };

    struct PiperConfig {
        std::string modelPath;
        std::string modelConfigPath;
    };

    class PiperSystem {
    public:
        bool loadModel(const std::string& model, const std::string& config) {
            LOGI("Loading Piper model from: %s", model.c_str());
            // Real implementation would load the ONNX session here
            return true;
        }

        bool synthesize(const std::string& text, const std::string& outputPath, const SynthesisConfig& config) {
            LOGI("Synthesizing text: '%s'", text.c_str());
            LOGI("Output: %s", outputPath.c_str());
            LOGI("Config: speed=%.2f, noise=%.2f, noiseW=%.2f, silence=%.2f",
                 config.lengthScale, config.noiseScale, config.noiseW, config.sentenceSilenceSeconds);
            // Real implementation would pass this config to the piper::textToAudioFile function
            return true;
        }
    };
}

// Global instance to hold the Piper session
static piper::PiperSystem g_piperSystem;

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

    bool success = g_piperSystem.loadModel(modelPath, configPath);
    return success ? JNI_TRUE : JNI_FALSE;
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

    piper::SynthesisConfig config;
    config.lengthScale = lengthScale;
    config.noiseScale = noiseScale;
    config.noiseW = noiseW;
    config.sentenceSilenceSeconds = sentenceSilence;

    bool success = g_piperSystem.synthesize(text, outputPath, config);

    // If we wanted to actually write a mock file from C++ for tests when real libpiper isn't linked:
    if (success) {
        FILE* f = fopen(outputPath.c_str(), "w");
        if (f) {
            fprintf(f, "NATIVE C++ MOCK WAV AUDIO DATA FOR: %s\n", text.c_str());
            fprintf(f, "Settings:\nLength Scale: %.3f\nNoise Scale: %.3f\nNoise W: %.3f\nSentence Silence: %.3f\n",
                    lengthScale, noiseScale, noiseW, sentenceSilence);
            fclose(f);
        } else {
            LOGE("Failed to write to %s", outputPath.c_str());
            return JNI_FALSE;
        }
    }

    return success ? JNI_TRUE : JNI_FALSE;
}
