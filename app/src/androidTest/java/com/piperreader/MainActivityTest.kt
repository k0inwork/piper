package com.piperreader

import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.action.ViewActions.click
import androidx.test.espresso.assertion.ViewAssertions.matches
import androidx.test.espresso.matcher.ViewMatchers.withId
import androidx.test.espresso.matcher.ViewMatchers.withText
import androidx.test.ext.junit.rules.ActivityScenarioRule
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.hamcrest.CoreMatchers.containsString
import android.os.Environment
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File

@RunWith(AndroidJUnit4::class)
class MainActivityTest {

    @get:Rule
    val activityRule = ActivityScenarioRule(MainActivity::class.java)

    @Test
    fun testTtsFlowAndFileGeneration() {
        // Click load text
        onView(withId(R.id.btnLoadText)).perform(click())

        // Ensure text is loaded (it contains Russian text)
        onView(withId(R.id.textView)).check(matches(withText(containsString("Это тестовый русский текст"))))

        // Click TTS
        onView(withId(R.id.btnTtsToFile)).perform(click())

        // Check success message
        onView(withId(R.id.textView)).check(matches(withText(containsString("Success! Audio saved to"))))

        // Verify the file was created in the app's external files directory
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
        val outputFile = File(downloadsDir, "output.wav")

        // Ensure file actually exists
        assertTrue("Output WAV file was not created", outputFile.exists())
        assertTrue("Output WAV file is empty", outputFile.length() > 0)

        val fileBytes = outputFile.readBytes()
        val fileContentString = String(fileBytes, Charsets.UTF_8)

        assertTrue(
            "Output WAV file is too small to be a valid audio file (size: ${outputFile.length()}). Content:\n$fileContentString",
            outputFile.length() > 300
        )

        // Read the first few bytes to check for a valid WAV header (RIFF ... WAVEfmt)
        // rather than the mock text file
        if (fileBytes.size >= 12) {
            val headerString = String(fileBytes, 0, 12, Charsets.US_ASCII)
            assertTrue(
                "File does not have a valid RIFF/WAVE header. Found: $headerString. Content:\n$fileContentString",
                headerString.startsWith("RIFF") && headerString.contains("WAVE")
            )
        } else {
            assertTrue("File is too small to even contain a header. Content:\n$fileContentString", false)
        }

        // Also explicitly ensure it doesn't contain the "MOCK WAV AUDIO DATA" text string
        assertTrue("File contains mock text data instead of real audio", !fileContentString.contains("MOCK WAV AUDIO DATA"))
    }
}
