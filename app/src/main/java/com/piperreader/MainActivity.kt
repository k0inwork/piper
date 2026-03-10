package com.piperreader

import android.os.Bundle
import android.os.Environment
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.SeekBar
import android.widget.Spinner
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var textView: TextView
    private lateinit var btnLoadText: Button
    private lateinit var btnTtsToFile: Button

    private lateinit var spinnerVoice: Spinner
    private lateinit var tvLengthScale: TextView
    private lateinit var sbLengthScale: SeekBar
    private lateinit var tvNoiseScale: TextView
    private lateinit var sbNoiseScale: SeekBar
    private lateinit var tvNoiseW: TextView
    private lateinit var sbNoiseW: SeekBar
    private lateinit var tvSentenceSilence: TextView
    private lateinit var sbSentenceSilence: SeekBar

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        textView = findViewById(R.id.textView)
        btnLoadText = findViewById(R.id.btnLoadText)
        btnTtsToFile = findViewById(R.id.btnTtsToFile)

        spinnerVoice = findViewById(R.id.spinnerVoice)
        tvLengthScale = findViewById(R.id.tvLengthScale)
        sbLengthScale = findViewById(R.id.sbLengthScale)
        tvNoiseScale = findViewById(R.id.tvNoiseScale)
        sbNoiseScale = findViewById(R.id.sbNoiseScale)
        tvNoiseW = findViewById(R.id.tvNoiseW)
        sbNoiseW = findViewById(R.id.sbNoiseW)
        tvSentenceSilence = findViewById(R.id.tvSentenceSilence)
        sbSentenceSilence = findViewById(R.id.sbSentenceSilence)

        setupUI()
        copyAssetsToInternalStorage()

        btnLoadText.setOnClickListener {
            loadText()
        }

        btnTtsToFile.setOnClickListener {
            ttsToFile()
        }
    }

    private fun copyAssetsToInternalStorage() {
        try {
            val assetsPath = "piper_voices"
            val destDir = File(filesDir, assetsPath)
            if (!destDir.exists()) {
                destDir.mkdirs()
            }

            val files = assets.list(assetsPath) ?: return
            for (filename in files) {
                val outFile = File(destDir, filename)
                if (!outFile.exists()) {
                    assets.open("$assetsPath/$filename").use { inStream ->
                        outFile.outputStream().use { outStream ->
                            inStream.copyTo(outStream)
                        }
                    }
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
            Toast.makeText(this, "Failed to copy voice models", Toast.LENGTH_SHORT).show()
        }
    }

    private fun setupUI() {
        val voices = arrayOf("Denis", "Dmitri", "Irina", "Ruslan")
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, voices)
        spinnerVoice.adapter = adapter

        setupSeekBar(sbLengthScale, tvLengthScale, "Speed (length_scale)", 100f)
        setupSeekBar(sbNoiseScale, tvNoiseScale, "Noise (noise_scale)", 100f)
        setupSeekBar(sbNoiseW, tvNoiseW, "Duration Variance (noise_w)", 100f)
        setupSeekBar(sbSentenceSilence, tvSentenceSilence, "Pause (sentence_silence)", 100f)
    }

    private fun setupSeekBar(seekBar: SeekBar, textView: TextView, label: String, scale: Float) {
        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                val value = progress / scale
                textView.text = "$label: ${String.format("%.3f", value)}"
            }
            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })
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

        // Retrieve settings from UI
        val selectedVoice = spinnerVoice.selectedItem.toString()
        val lengthScale = sbLengthScale.progress / 100f
        val noiseScale = sbNoiseScale.progress / 100f
        val noiseW = sbNoiseW.progress / 100f
        val sentenceSilence = sbSentenceSilence.progress / 100f

        // Initialize Piper with the selected voice model
        val internalVoicesDir = File(filesDir, "piper_voices")
        val modelFile = File(internalVoicesDir, "ru_RU-${selectedVoice.lowercase()}-medium.onnx")
        val configFile = File(internalVoicesDir, "ru_RU-${selectedVoice.lowercase()}-medium.onnx.json")

        if (!modelFile.exists()) {
            Toast.makeText(this, "Model for $selectedVoice not found locally.", Toast.LENGTH_SHORT).show()
            return
        }

        PiperTTS.initPiper(modelFile.absolutePath, configFile.absolutePath)

        // Output file in public Downloads directory so it survives test uninstalls
        val downloadsDir = getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS)
        val outputFile = File(downloadsDir, "output.wav")

        val success = PiperTTS.synthesizeToFile(
            textToRead,
            outputFile.absolutePath,
            lengthScale,
            noiseScale,
            noiseW,
            sentenceSilence
        )

        if (success) {
            textView.text = "Success! Audio saved to: ${outputFile.absolutePath}"
            Toast.makeText(this, "TTS Complete!", Toast.LENGTH_SHORT).show()
        } else {
            Toast.makeText(this, "TTS Failed", Toast.LENGTH_SHORT).show()
        }
    }
}
