#include <jni.h>
#include <string>

// JNI Stub

extern "C" JNIEXPORT jstring JNICALL
Java_com_piperreader_PiperTTS_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
