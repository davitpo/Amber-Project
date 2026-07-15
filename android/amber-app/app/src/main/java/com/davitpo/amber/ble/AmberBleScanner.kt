package com.davitpo.amber.ble

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log

enum class StopReason {
    FOUND,
    TIMEOUT,
    MANUAL,
    LIFECYCLE
}

class AmberBleScanner(private val context: Context, private val listener: Listener) {

    interface Listener {
        fun onScanStarted()
        fun onAmberFound(device: BluetoothDevice, rssi: Int)
        fun onScanStopped(reason: StopReason)
        fun onScanError(message: String)
    }

    private val bluetoothManager: BluetoothManager? by lazy {
        context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
    }

    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        bluetoothManager?.adapter
    }

    private val bleScanner: BluetoothLeScanner? by lazy {
        bluetoothAdapter?.bluetoothLeScanner
    }

    private var isScanning = false
    private val handler = Handler(Looper.getMainLooper())
    private val timeoutRunnable = Runnable { stopScan(StopReason.TIMEOUT) }

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            if (result == null) return
            
            val device = result.device
            val advertisedName = result.scanRecord?.deviceName ?: device.name ?: ""

            if (advertisedName.equals("Amber Clock", ignoreCase = true)) {
                Log.d("AmberBLE", "SCAN FOUND name=$advertisedName address=${device.address} rssi=${result.rssi}")
                handler.removeCallbacks(timeoutRunnable)
                listener.onAmberFound(device, result.rssi)
                stopScan(StopReason.FOUND)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e("AmberBLE", "SCAN ERROR code=$errorCode")
            isScanning = false
            handler.removeCallbacks(timeoutRunnable)
            listener.onScanError("Scan failed with error code: $errorCode")
        }
    }

    fun isBleAvailable(): Boolean {
        return bluetoothAdapter != null && bleScanner != null
    }

    fun isBleEnabled(): Boolean {
        return bluetoothAdapter?.isEnabled == true
    }

    @SuppressLint("MissingPermission")
    fun startScan(): Boolean {
        if (isScanning) return true
        
        val scanner = bleScanner
        if (scanner == null) {
            listener.onScanError("BLE LeScanner not available")
            return false
        }

        Log.d("AmberBLE", "SCAN START")
        isScanning = true
        listener.onScanStarted()
        
        handler.postDelayed(timeoutRunnable, 10000)
        scanner.startScan(scanCallback)
        return true
    }

    @SuppressLint("MissingPermission")
    fun stopScan(reason: StopReason = StopReason.MANUAL) {
        if (!isScanning) return
        
        Log.d("AmberBLE", "SCAN STOP reason=$reason")
        isScanning = false
        handler.removeCallbacks(timeoutRunnable)
        bleScanner?.stopScan(scanCallback)
        listener.onScanStopped(reason)
    }
}
