package com.davitpo.amber

import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()
        setContentView(R.layout.activity_main)

        val mainView = findViewById<android.view.View>(R.id.main)
        val statusText = findViewById<TextView>(R.id.statusText)
        val responseText = findViewById<TextView>(R.id.responseText)
        val scanButton = findViewById<Button>(R.id.scanButton)

        ViewCompat.setOnApplyWindowInsetsListener(mainView) { view, insets ->
            val systemBars =
                insets.getInsets(WindowInsetsCompat.Type.systemBars())

            view.setPadding(
                systemBars.left + 24,
                systemBars.top + 24,
                systemBars.right + 24,
                systemBars.bottom + 24
            )

            insets
        }

        scanButton.setOnClickListener {
            statusText.setText(R.string.status_scanning)
            responseText.setText(R.string.scan_placeholder)
        }
    }
}