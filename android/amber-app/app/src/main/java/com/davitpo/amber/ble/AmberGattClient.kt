package com.davitpo.amber.ble

import android.annotation.SuppressLint
import android.bluetooth.*
import android.content.Context
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.util.Log
import java.util.UUID

object AmberUuids {
    val SERVICE_UUID: UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef0")
    val RX_CHAR_UUID: UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef1")
    val TX_CHAR_UUID: UUID = UUID.fromString("12345678-1234-5678-1234-56789abcdef2")
    val CCCD_UUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
}

enum class GattState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCOVERING_SERVICES,
    ENABLING_NOTIFICATIONS,
    READY,
    DISCONNECTING
}

class AmberGattClient(private val context: Context, private val listener: Listener) {

    interface Listener {
        fun onConnecting(device: BluetoothDevice)
        fun onConnected(device: BluetoothDevice)
        fun onEnablingNotifications()
        fun onNotificationsEnabled()
        fun onNotificationReceived(data: ByteArray)
        fun onAcpWriteComplete()
        fun onAcpWriteFailed(status: Int)
        fun onDisconnected()
        fun onConnectionError(message: String)
    }

    private var bluetoothGatt: BluetoothGatt? = null
    private val handler = Handler(Looper.getMainLooper())
    
    var state: GattState = GattState.DISCONNECTED
        private set

    private var rxCharacteristic: BluetoothGattCharacteristic? = null
    private var txCharacteristic: BluetoothGattCharacteristic? = null

    // Track write pending states cleanly to serialize outstanding operations
    var isWritePending = false
        private set

    // Send ACP command byte array to the validated RX characteristic
    @SuppressLint("MissingPermission")
    fun writeAcpCommand(data: ByteArray): Boolean {
        if (state != GattState.READY) return false
        val gatt = bluetoothGatt ?: return false
        val rxChar = rxCharacteristic ?: return false
        if (isWritePending) return false

        isWritePending = true
        val defensiveCopy = data.clone()

        val rxProps = rxChar.properties
        val writeType = if ((rxProps and BluetoothGattCharacteristic.PROPERTY_WRITE) != 0) {
            BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        } else if ((rxProps and BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0) {
            BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
        } else {
            BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        }

        Log.d("AmberACP", "ACP WRITE START len=${defensiveCopy.size}")

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            val resCode = gatt.writeCharacteristic(rxChar, defensiveCopy, writeType)
            val success = resCode == BluetoothStatusCodes.SUCCESS
            if (!success) {
                isWritePending = false
            }
            return success
        } else {
            @Suppress("DEPRECATION")
            rxChar.value = defensiveCopy
            @Suppress("DEPRECATION")
            rxChar.writeType = writeType
            val success = gatt.writeCharacteristic(rxChar)
            if (!success) {
                isWritePending = false
            }
            return success
        }
    }

    // Send ASCII payload string to the validated RX characteristic
    @SuppressLint("MissingPermission")
    fun writeRxCommand(command: String): Boolean {
        return writeAcpCommand(command.toByteArray(Charsets.UTF_8))
    }

    private val gattCallback = object : BluetoothGattCallback() {
        @SuppressLint("MissingPermission")
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                val errMsg = "GATT connection failure status: $status"
                Log.e("AmberGATT", "ERROR=$errMsg")
                closeGatt()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d("AmberGATT", "STATE CONNECTED status=$status")
                state = GattState.CONNECTED
                postToMain { listener.onConnected(gatt!!.device) }
                
                state = GattState.DISCOVERING_SERVICES
                Log.d("AmberGATT", "DISCOVERY START")
                val startOk = gatt?.discoverServices() ?: false
                if (!startOk) {
                    val discoErrMsg = "ERROR=SERVICE_DISCOVERY_START_FAILED"
                    Log.e("AmberGATT", discoErrMsg)
                    disconnect()
                    postToMain { listener.onConnectionError(discoErrMsg) }
                }
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d("AmberGATT", "STATE DISCONNECTED status=$status")
                closeGatt()
                postToMain { listener.onDisconnected() }
            }
        }

        @SuppressLint("MissingPermission")
        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            Log.d("AmberGATT", "DISCOVERY COMPLETE status=$status")
            if (status != BluetoothGatt.GATT_SUCCESS) {
                val errMsg = "Service discovery failure status: $status"
                Log.e("AmberGATT", "ERROR=$errMsg")
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }

            val service = gatt?.getService(AmberUuids.SERVICE_UUID)
            if (service == null) {
                val errMsg = "ERROR=AMBER_SERVICE_NOT_FOUND"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }
            Log.d("AmberGATT", "SERVICE FOUND")

            val rxChar = service.getCharacteristic(AmberUuids.RX_CHAR_UUID)
            if (rxChar == null) {
                val errMsg = "ERROR=RX_CHARACTERISTIC_NOT_FOUND"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }
            Log.d("AmberGATT", "RX FOUND properties=${rxChar.properties}")

            val txChar = service.getCharacteristic(AmberUuids.TX_CHAR_UUID)
            if (txChar == null) {
                val errMsg = "ERROR=TX_CHARACTERISTIC_NOT_FOUND"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }
            Log.d("AmberGATT", "TX FOUND properties=${txChar.properties}")

            // Validate characteristic write and notify capabilities
            val hasRxWritable = (rxChar.properties and (BluetoothGattCharacteristic.PROPERTY_WRITE or
                    BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) != 0
            val hasTxNotifiable = (txChar.properties and BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0

            if (!hasRxWritable) {
                val errMsg = "Validation failed: RX characteristic is not writable"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }

            if (!hasTxNotifiable) {
                val errMsg = "Validation failed: TX characteristic is not notifiable"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }

            rxCharacteristic = rxChar
            txCharacteristic = txChar

            enableTxNotifications(gatt!!)
        }

        override fun onDescriptorWrite(gatt: BluetoothGatt?, descriptor: BluetoothGattDescriptor?, status: Int) {
            if (descriptor == null || state != GattState.ENABLING_NOTIFICATIONS) return

            if (descriptor.uuid == AmberUuids.CCCD_UUID && descriptor.characteristic?.uuid == AmberUuids.TX_CHAR_UUID) {
                Log.d("AmberGATT", "CCCD WRITE COMPLETE status=$status")
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    state = GattState.READY
                    Log.d("AmberGATT", "NOTIFICATIONS READY")
                    postToMain { listener.onNotificationsEnabled() }
                } else {
                    val errMsg = "ERROR=CCCD_WRITE_FAILED status=$status"
                    Log.e("AmberGATT", errMsg)
                    disconnect()
                    postToMain { listener.onConnectionError(errMsg) }
                }
            }
        }

        override fun onCharacteristicWrite(gatt: BluetoothGatt?, characteristic: BluetoothGattCharacteristic?, status: Int) {
            if (characteristic == null || characteristic.uuid != AmberUuids.RX_CHAR_UUID) return
            
            isWritePending = false
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d("AmberACP", "ACP WRITE COMPLETE status=$status")
                postToMain { listener.onAcpWriteComplete() }
            } else {
                Log.e("AmberACP", "ACP WRITE FAILED status=$status")
                postToMain { listener.onAcpWriteFailed(status) }
            }
        }

        override fun onCharacteristicChanged(gatt: BluetoothGatt?, characteristic: BluetoothGattCharacteristic?) {
            if (characteristic == null) return
            if (characteristic.uuid == AmberUuids.TX_CHAR_UUID) {
                val value = characteristic.value ?: return
                val defensiveCopy = value.clone()
                Log.d("AmberGATT", "NOTIFY RX len=${defensiveCopy.size}")
                postToMain { listener.onNotificationReceived(defensiveCopy) }
            }
        }

        // Support safe API 33+ memory-safe changes callbacks
        override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, value: ByteArray) {
            if (characteristic.uuid == AmberUuids.TX_CHAR_UUID) {
                val defensiveCopy = value.clone()
                Log.d("AmberGATT", "NOTIFY RX len=${defensiveCopy.size}")
                postToMain { listener.onNotificationReceived(defensiveCopy) }
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun enableTxNotifications(gatt: BluetoothGatt) {
        val txChar = txCharacteristic ?: return
        
        Log.d("AmberGATT", "NOTIFY LOCAL ENABLE START")
        state = GattState.ENABLING_NOTIFICATIONS
        postToMain { listener.onEnablingNotifications() }

        val localSetOk = gatt.setCharacteristicNotification(txChar, true)
        if (!localSetOk) {
            val errMsg = "ERROR=LOCAL_NOTIFICATION_ENABLE_FAILED"
            Log.e("AmberGATT", errMsg)
            disconnect()
            postToMain { listener.onConnectionError(errMsg) }
            return
        }
        Log.d("AmberGATT", "NOTIFY LOCAL ENABLED")

        val cccd = txChar.getDescriptor(AmberUuids.CCCD_UUID)
        if (cccd == null) {
            val errMsg = "ERROR=TX_CCCD_NOT_FOUND"
            Log.e("AmberGATT", errMsg)
            disconnect()
            postToMain { listener.onConnectionError(errMsg) }
            return
        }
        Log.d("AmberGATT", "CCCD FOUND")

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            Log.d("AmberGATT", "CCCD WRITE START api=33+")
            val resCode = gatt.writeDescriptor(cccd, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)
            if (resCode != BluetoothStatusCodes.SUCCESS) {
                val errMsg = "ERROR=CCCD_WRITE_INIT_FAILED code=$resCode"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }
            Log.d("AmberGATT", "CCCD WRITE ACCEPTED")
        } else {
            Log.d("AmberGATT", "CCCD WRITE START api=legacy")
            @Suppress("DEPRECATION")
            cccd.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            val ok = gatt.writeDescriptor(cccd)
            if (!ok) {
                val errMsg = "ERROR=CCCD_WRITE_INIT_FAILED"
                Log.e("AmberGATT", errMsg)
                disconnect()
                postToMain { listener.onConnectionError(errMsg) }
                return
            }
            Log.d("AmberGATT", "CCCD WRITE ACCEPTED")
        }
    }

    @SuppressLint("MissingPermission")
    fun connect(device: BluetoothDevice): Boolean {
        if (state != GattState.DISCONNECTED) return false
        
        Log.d("AmberGATT", "CONNECT START address=${device.address}")
        state = GattState.CONNECTING
        listener.onConnecting(device)
        
        bluetoothGatt = device.connectGatt(context, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
        return true
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        if (state == GattState.DISCONNECTED || state == GattState.DISCONNECTING) return
        
        Log.d("AmberGATT", "DISCONNECT START")
        state = GattState.DISCONNECTING
        bluetoothGatt?.disconnect()
    }

    @SuppressLint("MissingPermission")
    private fun closeGatt() {
        bluetoothGatt?.close()
        bluetoothGatt = null
        rxCharacteristic = null
        txCharacteristic = null
        isWritePending = false
        state = GattState.DISCONNECTED
    }

    private fun postToMain(action: () -> Unit) {
        handler.post(action)
    }
}
