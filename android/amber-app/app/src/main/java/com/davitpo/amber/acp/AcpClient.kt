package com.davitpo.amber.acp

import java.nio.charset.StandardCharsets

class AcpClient(private val transport: Transport, private val listener: Listener) {

    interface Transport {
        fun isReady(): Boolean
        fun write(data: ByteArray): Boolean
    }

    interface Listener {
        fun onCommandAccepted(command: String)
        fun onCommandRejected(message: String)
        fun onResponseChunk(text: String)
    }

    fun sendCommand(rawCommand: String): Boolean {
        // Enforce command formatting checks before transferring payload bytes
        val trimmed = rawCommand.trim()
        if (trimmed.isEmpty()) {
            listener.onCommandRejected("Command is empty")
            return false
        }

        val bytes = trimmed.toByteArray(StandardCharsets.UTF_8)
        if (bytes.size > 64) {
            listener.onCommandRejected("Command exceeds 64 bytes")
            return false
        }

        // Reject commands containing control segments
        for (b in bytes) {
            val charVal = b.toInt().toChar()
            if (charVal == '\r' || charVal == '\n' || charVal == '\u0000') {
                listener.onCommandRejected("Command contains CR, LF or NUL characters")
                return false
            }
        }

        if (!transport.isReady()) {
            listener.onCommandRejected("Transport is not ready")
            return false
        }

        val success = transport.write(bytes)
        if (success) {
            listener.onCommandAccepted(trimmed)
            return true
        } else {
            listener.onCommandRejected("Write initiation failed")
            return false
        }
    }

    fun receiveNotificationBytes(data: ByteArray) {
        val decoded = String(data, StandardCharsets.UTF_8)
        listener.onResponseChunk(decoded)
    }
}
