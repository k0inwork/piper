package com.piperreader

import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var textView: TextView
    private lateinit var btnLoadText: Button
    private lateinit var btnTtsToFile: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        textView = findViewById(R.id.textView)
        btnLoadText = findViewById(R.id.btnLoadText)
        btnTtsToFile = findViewById(R.id.btnTtsToFile)

        btnLoadText.setOnClickListener {
            loadText()
        }

        btnTtsToFile.setOnClickListener {
            ttsToFile()
        }
    }

    private fun loadText() {
        try {
            val text = assets.open("test_ru.txt").bufferedReader().use { it.readText() }
            textView.text = text
        } catch (e: Exception) {
            textView.text = "Error loading text: ${e.message}"
        }
    }

    private fun ttsToFile() {
        val textToRead = textView.text.toString()
        if (textToRead.isBlank() || textToRead.startsWith("Error")) {
            Toast.makeText(this, "Please load text first", Toast.LENGTH_SHORT).show()
            return
        }

        // Output file in cache directory
        val outputFile = File(cacheDir, "output.wav")

        val success = PiperTTS.synthesizeToFile(textToRead, outputFile.absolutePath)

        if (success) {
            textView.text = "Success! Audio saved to: ${outputFile.absolutePath}"
            Toast.makeText(this, "TTS Complete!", Toast.LENGTH_SHORT).show()
        } else {
            Toast.makeText(this, "TTS Failed", Toast.LENGTH_SHORT).show()
        }
    }
}
