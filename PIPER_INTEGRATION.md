# Piper TTS Integration Analysis for PiperReader

This document details the requirements to replace the current mock TTS implementation in `PiperReader` with a fully functional, offline Piper TTS engine, specifically focusing on voice generation quality and UI configuration.

## 1. Current Status of the Application

*   **UI:** The application currently has a barebones UI (`activity_main.xml`) with a `TextView` to display loaded text, a "Load Text" button, and a "TTS to File" button.
*   **TTS Implementation (`PiperTTS.kt`):** The application relies on a mock implementation for Text-To-Speech. The `synthesizeToFile` function simply writes a hardcoded string ("MOCK WAV AUDIO DATA FOR: [text]") to a `.wav` file instead of synthesizing actual audio.
*   **Native Code (`native-lib.cpp`):** The JNI setup exists but only contains a stub `stringFromJNI` function. No real interaction with the Piper C++ library occurs.
*   **Project Configuration:** The application is set up with CMake to build native JNI code, and it relies on extracting pre-compiled JNI libraries (`libpiper_android.so`) which must be packaged by Gradle.

## 2. Requirements to Extend with Real Piper TTS

To move from the mock implementation to the real Piper TTS system, the following steps are required:

### 2.1 Native JNI Integration
*   **Update C++ Wrapper:** The `native-lib.cpp` file must be updated to include headers from the Piper C++ library (`piper.hpp`, etc.).
*   **JNI Method Implementation:** We need to implement JNI functions that map to `PiperTTS.kt`. Crucially, an initialization function to load the ONNX model and JSON config, and a synthesis function `synthesizeToFile(text, outputPath)`.
*   **Piper Configuration Struct:** The JNI code must be capable of receiving synthesis parameters (speed, noise scale, etc.) from Kotlin and populating the `piper::SynthesisConfig` struct before calling the `piper::textToAudioFile` or `piper::textToAudio` functions.

### 2.2 Kotlin Layer (`PiperTTS.kt`)
*   **Load Library:** Ensure `System.loadLibrary("native-lib")` (or the respective library name) is called.
*   **External Methods:** Declare `external fun` for initialization (e.g., `initPiper(modelPath: String, configPath: String, dataPath: String): Long`) and synthesis. The synthesis function should accept advanced settings.

### 2.3 Model Management
*   **Asset/File Management:** ONNX models (`.onnx`) and their corresponding configuration files (`.onnx.json`) must be available on the device's file system, as the native Piper library requires file paths. They should be copied from `assets` to internal storage (`context.filesDir`) or downloaded on demand.

## 3. Piper TTS Settings for Voice Quality

Piper TTS allows tweaking several parameters during inference to modify the generated speech characteristics. Exposing these settings is vital for the best user experience.

| Parameter | Type | Default | Description | Impact on Quality |
| :--- | :--- | :--- | :--- | :--- |
| **`length_scale`** | Float | 1.0 | Controls the overall speed of the speech. | Lower values (<1.0) make speech faster; higher values (>1.0) make speech slower. Extreme values can cause distortion. |
| **`noise_scale`** | Float | 0.667 | Controls the variance (noise) in the audio generation process (phoneme generation). | Affects the emotional tone and naturalness. Higher values increase variability but might introduce audio artifacts. Lower values sound more monotonous and robotic. |
| **`noise_w`** | Float | 0.8 | Controls the variance in phoneme duration. | Affects the rhythm and pacing of the speech. Finding the right balance prevents the speech from sounding artificially uniform. |
| **`sentence_silence`** | Float | 0.2 | The amount of silence (in seconds) inserted between sentences. | Improves the readability of long texts by providing natural pauses, reducing listener fatigue. |

## 4. GUI Implementation for Settings

To allow users to customize their TTS experience, the UI must be expanded.

### 4.1 UI Components

We should add a settings panel (either in a bottom sheet, a settings activity, or inline in the main view) with the following controls:

*   **Voice Selection (Dropdown/Spinner):**
    *   Allows switching between the supported voices (Denis, Dmitri, Irina, Ruslan).
    *   **Action:** Triggers the JNI layer to unload the current model and initialize the newly selected ONNX model and config.
*   **Speaking Rate / Speed (`length_scale`):**
    *   **Component:** `Slider` (Material Design) or `SeekBar`.
    *   **Range:** ~0.5 (Fast) to ~2.0 (Slow), with 1.0 being the center/default. Note that the slider values need to be mapped inversely if we label it "Speed" (i.e., higher speed = lower `length_scale`).
*   **Expressiveness / Variability (`noise_scale`):**
    *   **Component:** `Slider`.
    *   **Range:** 0.0 to 1.0, defaulting to 0.667.
*   **Rhythm Variance (`noise_w`):**
    *   **Component:** `Slider`.
    *   **Range:** 0.0 to 1.0, defaulting to 0.8.
*   **Pause Duration (`sentence_silence`):**
    *   **Component:** `Slider`.
    *   **Range:** 0.0 to 2.0 seconds, defaulting to 0.2.

### 4.2 Passing Settings to Piper

1.  **State Management:** The Kotlin application should maintain the current state of these parameters (e.g., using `SharedPreferences` or a `ViewModel`).
2.  **Updating Synthesis Call:** The `PiperTTS.synthesizeToFile` method signature should be updated to accept a `SynthesisConfig` data class or pass the parameters directly:
    ```kotlin
    external fun synthesizeToFile(
        text: String,
        outputPath: String,
        lengthScale: Float,
        noiseScale: Float,
        noiseW: Float,
        sentenceSilence: Float
    ): Boolean
    ```
3.  **JNI Handling:** In the C++ JNI implementation, extract these floats and apply them to the Piper context before calling the generation routine.