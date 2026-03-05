# Design Document: Offline EPUB/PDF eReader with Piper TTS

## 1. Overview

**Project Name:** PiperReader
**Goal:** Build an eReader for EPUB and PDF files that:

* Reads text aloud using **offline Piper TTS**.
* Tracks words on-screen for synchronized highlighting.
* Supports **4 Russian voices** (Denis, Dmitri, Irina, Ruslan).
* Works entirely offline on Android devices.

**Target Users:** Russian-speaking readers who want immersive reading with voice narration.

---

## 2. Features

| Feature                | Description                                                           |
| ---------------------- | --------------------------------------------------------------------- |
| EPUB & PDF rendering   | Display EPUB and PDF pages with word-level tracking.                  |
| Word-level TTS         | Track visible words and synthesize them in real-time.                 |
| Voice selection        | Allow user to choose between 4 Russian voices.                        |
| Offline operation      | Use Piper ONNX models to generate speech without internet.            |
| Adjustable speed/pitch | Optional TTS controls for personalized reading speed and voice pitch. |
| Highlighting           | Highlight current spoken word in sync with audio.                     |
| Page navigation        | Standard eReader navigation (scroll, jump to page, search).           |
| Bookmarking            | Save reading positions and TTS state.                                 |

---

## 3. Supported Piper Voices

| Voice  | ONNX Model File            | Hugging Face Link                                                                                                 |
| ------ | -------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Denis  | `ru_RU-denis-medium.onnx`  | [Download](https://huggingface.co/rhasspy/piper-voices/blob/main/ru/ru_RU/denis/medium/ru_RU-denis-medium.onnx)   |
| Dmitri | `ru_RU-dmitri-medium.onnx` | [Download](https://huggingface.co/rhasspy/piper-voices/blob/main/ru/ru_RU/dmitri/medium/ru_RU-dmitri-medium.onnx) |
| Irina  | `ru_RU-irina-medium.onnx`  | [Download](https://huggingface.co/rhasspy/piper-voices/blob/main/ru/ru_RU/irina/medium/ru_RU-irina-medium.onnx)   |
| Ruslan | `ru_RU-ruslan-medium.onnx` | [Download](https://huggingface.co/rhasspy/piper-voices/blob/main/ru/ru_RU/ruslan/medium/ru_RU-ruslan-medium.onnx) |

> Models are stored locally in `appFilesDir/piper_voices/` for offline usage.

---

## 4. Architecture

### 4.1 High-Level Components

```
+------------------+
| UI Layer         |
|------------------|
| Page Renderer    | <--- EPUB / PDF rendering with word positions
| Word Highlighter |
| TTS Controls     |
+------------------+
         |
         v
+------------------+
| TTS Engine Layer |
|------------------|
| Piper ONNX Loader| <-- Loads ONNX models at startup
| Speech Synthesizer| <-- Generates audio for current word/text
+------------------+
         |
         v
+------------------+
| Audio Output     |
|------------------|
| Plays synthesized speech
| Handles sync with UI
+------------------+
```

---

### 4.2 Data Flow

1. **Page Rendered** → Extract words and coordinates.
2. **User taps “Read Aloud”** → Select voice → Initialize Piper ONNX model session.
3. **Word Tracking Loop**:

   * Fetch visible word on screen.
   * Generate speech for that word using Piper.
   * Play audio → Highlight word.
4. **Move to next word/page** automatically or by user navigation.

---

## 5. ONNX Model Handling

* **Startup:** Preload all 4 Russian voices using ONNX Runtime:

```python
import onnxruntime as ort

voice_files = {
    "denis": "piper_voices/ru_RU-denis-medium.onnx",
    "dmitri": "piper_voices/ru_RU-dmitri-medium.onnx",
    "irina": "piper_voices/ru_RU-irina-medium.onnx",
    "ruslan": "piper_voices/ru_RU-ruslan-medium.onnx"
}

sessions = {name: ort.InferenceSession(path) for name, path in voice_files.items()}
```

* **Memory Optimization:** Lazy-load models if device RAM is limited.
* **Offline:** No network dependency; ONNX models produce audio locally.

---

## 6. EPUB & PDF Word Tracking

1. Parse text layout using:

   * EPUB: [epub.js](https://github.com/futurepress/epub.js) / Android EPUB parser
   * PDF: [PDFBox](https://pdfbox.apache.org/) / MuPDF
2. Map **word positions** on the screen (x, y coordinates).
3. When TTS plays a word → highlight the word in the UI in real-time.
4. Optional: allow **speed adjustment** while keeping word sync.

---

## 7. Performance & Optimization

* **ONNX Runtime GPU/NNAPI support** (if available) for faster TTS.
* **Pre-generate sentence chunks** to avoid lag between words.
* **Audio caching**: store synthesized words temporarily for repeated reads.
* **Memory management**: only keep necessary ONNX sessions loaded to conserve RAM.

---

## 8. UI / UX Considerations

* **Voice Picker**: Dropdown to select Denis, Dmitri, Irina, Ruslan.
* **Highlight Style**: Bold, underline, or colored background for current word.
* **Controls**: Play, pause, skip word, skip page.
* **Settings**: Reading speed, pitch, TTS voice selection, cache management.

---

## 9. Future Extensions

* Add more languages (download additional Piper ONNX voices).
* Support for **sentence-level synthesis** to reduce TTS calls.
* Gesture-based navigation with word sync.
* Integration with **dictionary lookup** for highlighted words.
