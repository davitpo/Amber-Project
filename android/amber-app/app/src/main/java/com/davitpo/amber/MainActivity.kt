package com.davitpo.amber

import android.Manifest
import android.bluetooth.BluetoothDevice
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.view.inputmethod.EditorInfo
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.davitpo.amber.acp.AcpClient
import com.davitpo.amber.ble.AmberBleScanner
import com.davitpo.amber.ble.AmberGattClient
import com.davitpo.amber.ble.GattState
import com.davitpo.amber.ble.StopReason

class MainActivity : AppCompatActivity() {

    private lateinit var tvStatus: TextView
    private lateinit var terminalText: TextView
    private lateinit var btnScan: Button
    private lateinit var btnConnect: Button
    private lateinit var acpCommandInput: EditText
    private lateinit var sendCommandButton: Button
    private lateinit var btnClearTerminal: Button

    private var bleScanner: AmberBleScanner? = null
    private var gattClient: AmberGattClient? = null
    private var acpClient: AcpClient? = null
    private var detectedDevice: BluetoothDevice? = null

    // Terminal History Bounded memory (16 KB max)
    private val terminalHistory = StringBuilder()
    private val maxHistoryChars = 16384

    // Register active permissions handler utilizing modern Activity Result API
    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val granted = permissions.entries.all { it.value }
        if (granted) {
            triggerBleScanning()
        } else {
            tvStatus.text = "Status: Permission denied"
            appendTerminalLine("! ERROR: Required Bluetooth/Location permission has been denied.")
            btnScan.isEnabled = true
            btnConnect.isEnabled = false
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Find targets directly from layouts leveraging exact matched string IDs
        tvStatus = findViewById<TextView>(resources.getIdentifier("tv_status", "id", packageName))
        terminalText = findViewById<TextView>(resources.getIdentifier("terminal_text", "id", packageName)
            .let { if (it != 0) it else resources.getIdentifier("terminalText", "id", packageName) })
        btnScan = findViewById<Button>(resources.getIdentifier("btn_scan", "id", packageName))
        btnConnect = findViewById<Button>(resources.getIdentifier("btn_connect", "id", packageName))
        
        acpCommandInput = findViewById<EditText>(resources.getIdentifier("acpCommandInput", "id", packageName))
        sendCommandButton = findViewById<Button>(resources.getIdentifier("sendCommandButton", "id", packageName))
        btnClearTerminal = findViewById<Button>(resources.getIdentifier("btnClearTerminal", "id", packageName))

        setCommandControlsEnabled(false)

        bleScanner = AmberBleScanner(this, object : AmberBleScanner.Listener {
            override fun onScanStarted() {
                tvStatus.text = "Status: Scanning…"
                appendTerminalLine("> SCAN START")
                btnScan.isEnabled = false
                btnConnect.isEnabled = false
            }

            override fun onAmberFound(device: BluetoothDevice, rssi: Int) {
                detectedDevice = device
                tvStatus.text = "Status: Amber Clock found"
                appendTerminalLine("< NAME=Amber Clock\nADDRESS=${device.address}\nRSSI=$rssi dBm")
                btnScan.isEnabled = true
                btnConnect.isEnabled = true
            }

            override fun onScanStopped(reason: StopReason) {
                btnScan.isEnabled = true
                if (reason == StopReason.TIMEOUT) {
                    tvStatus.text = getString(R.string.status_not_found)
                    appendTerminalLine("< Amber Clock was not found.")
                    btnConnect.isEnabled = false
                }
            }

            override fun onScanError(message: String) {
                tvStatus.text = getString(R.string.status_scan_failed)
                appendTerminalLine("! ERROR=$message")
                btnScan.isEnabled = true
                btnConnect.isEnabled = false
            }
        })

        // Initialize our isolated, main thread matching GATT client
        gattClient = AmberGattClient(this, object : AmberGattClient.Listener {
            override fun onConnecting(device: BluetoothDevice) {
                tvStatus.text = getString(R.string.status_connecting)
                appendTerminalLine("> CONNECT START address=${device.address}")
                btnScan.isEnabled = false
                btnConnect.isEnabled = false
                setCommandControlsEnabled(false)
            }

            override fun onConnected(device: BluetoothDevice) {
                tvStatus.text = getString(R.string.status_connected_discovering)
                appendTerminalLine("< STATE CONNECTED")
            }

            override fun onEnablingNotifications() {
                tvStatus.text = getString(R.string.status_enabling_notifications)
                appendTerminalLine("< DISCOVERED: SERVICE=FOUND, RX=READY, TX=READY")
            }

            override fun onNotificationsEnabled() {
                tvStatus.text = getString(R.string.status_amber_ready)
                appendTerminalLine("< READY FOR TERMINAL ACP SESSION")
                btnScan.isEnabled = false
                btnConnect.text = getString(R.string.action_disconnect)
                btnConnect.isEnabled = true
                setCommandControlsEnabled(true)
            }

            override fun onNotificationReceived(data: ByteArray) {
                // Pass notification bytes direct to local AcpClient processing stream
                acpClient?.receiveNotificationBytes(data)
            }

            override fun onAcpWriteComplete() {
                // Enable command controls again once characteristic write completes
                if (gattClient?.state == GattState.READY) {
                    sendCommandButton.isEnabled = true
                    acpCommandInput.requestFocus()
                }
            }

            override fun onAcpWriteFailed(status: Int) {
                appendTerminalLine("! ERROR=ACP_WRITE_FAILED STATUS=$status")
                if (gattClient?.state == GattState.READY) {
                    sendCommandButton.isEnabled = true
                    acpCommandInput.requestFocus()
                }
            }

            override fun onDisconnected() {
                tvStatus.text = "Status: Disconnected"
                appendTerminalLine("< Connection closed.")
                btnScan.isEnabled = true
                btnConnect.text = getString(R.string.action_connect)
                btnConnect.isEnabled = detectedDevice != null
                setCommandControlsEnabled(false)
            }

            override fun onConnectionError(message: String) {
                tvStatus.text = getString(R.string.status_connection_failed)
                appendTerminalLine("! ERROR: Connection failed - $message")
                btnScan.isEnabled = true
                btnConnect.text = getString(R.string.action_connect)
                btnConnect.isEnabled = detectedDevice != null
                setCommandControlsEnabled(false)
            }
        })

        // Instantiates transport matching gattClient interface specifications
        acpClient = AcpClient(object : AcpClient.Transport {
            override fun isReady(): Boolean {
                val gatt = gattClient ?: return false
                return gatt.state == GattState.READY && !gatt.isWritePending
            }

            override fun write(data: ByteArray): Boolean {
                val gatt = gattClient ?: return false
                return gatt.writeAcpCommand(data)
            }
        }, object : AcpClient.Listener {
            override fun onCommandAccepted(command: String) {
                appendTerminalLine("> $command")
                acpCommandInput.setText("")
                sendCommandButton.isEnabled = false // disable until write completed trigger
            }

            override fun onCommandRejected(message: String) {
                appendTerminalLine("! $message")
                sendCommandButton.isEnabled = true
            }

            override fun onResponseChunk(text: String) {
                if (text.isEmpty()) return

                // Treat each received Amber TX notification as one display line
                // Remove trailing CR, LF or whitespace formatting from the decoded chunk
                var cleaned = text
                while (cleaned.endsWith("\r") || cleaned.endsWith("\n")) {
                    cleaned = cleaned.substring(0, cleaned.length - 1)
                }

                if (cleaned.isNotEmpty()) {
                    appendTerminalLine("< $cleaned")
                }
            }
        })

        btnScan.setOnClickListener {
            evaluatePermissionsAndScan()
        }

        btnConnect.setOnClickListener {
            val client = gattClient ?: return@setOnClickListener
            if (client.state == GattState.DISCONNECTED) {
                val device = detectedDevice
                if (device != null) {
                    // Halts scanner, prevents overlapping runs
                    bleScanner?.stopScan(StopReason.MANUAL)
                    client.connect(device)
                }
            } else {
                client.disconnect()
            }
        }

        sendCommandButton.setOnClickListener {
            executeSendCommand()
        }

        acpCommandInput.setOnEditorActionListener { _, actionId, _ ->
            if (actionId == EditorInfo.IME_ACTION_SEND) {
                executeSendCommand()
                true
            } else {
                false
            }
        }

        btnClearTerminal.setOnClickListener {
            terminalHistory.setLength(0)
            terminalText.text = ""
        }
    }

    private fun executeSendCommand() {
        if (gattClient?.state != GattState.READY) return
        val cmd = acpCommandInput.text.toString()
        acpClient?.sendCommand(cmd)
    }

    private fun setCommandControlsEnabled(enabled: Boolean) {
        acpCommandInput.isEnabled = enabled
        sendCommandButton.isEnabled = enabled
    }

    private fun appendTerminalLine(line: String) {
        appendTerminalText(line + "\n")
    }

    private fun appendTerminalText(text: String) {
        terminalHistory.append(text)
        
        // Truncate bounded text history if it exceeds the 16 KB characters threshold limit
        if (terminalHistory.length > maxHistoryChars) {
            val overflow = terminalHistory.length - maxHistoryChars
            terminalHistory.delete(0, overflow)
        }
        
        terminalText.text = terminalHistory.toString()
    }

    private fun evaluatePermissionsAndScan() {
        val permissions = mutableListOf<String>()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            permissions.add(Manifest.permission.BLUETOOTH_SCAN)
            permissions.add(Manifest.permission.BLUETOOTH_CONNECT)
        } else {
            permissions.add(Manifest.permission.ACCESS_FINE_LOCATION)
        }

        val alreadyGranted = permissions.all {
            ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
        }

        if (alreadyGranted) {
            triggerBleScanning()
        } else {
            requestPermissionLauncher.launch(permissions.toTypedArray())
        }
    }

    private fun triggerBleScanning() {
        val scanner = bleScanner ?: return
        if (!scanner.isBleAvailable()) {
            tvStatus.text = "Status: Scan failed"
            appendTerminalLine("! ERROR=Bluetooth hardware or LE scanner not available")
            btnScan.isEnabled = true
            return
        }
        if (!scanner.isBleEnabled()) {
            tvStatus.text = "Status: Bluetooth disabled"
            appendTerminalLine("! ERROR=Bluetooth is currently disabled.")
            btnScan.isEnabled = true
            btnConnect.isEnabled = false
            return
        }
        scanner.startScan()
    }

    override fun onStop() {
        super.onStop()
        bleScanner?.stopScan(StopReason.LIFECYCLE)
    }

    override fun onDestroy() {
        super.onDestroy()
        gattClient?.disconnect()
    }
}