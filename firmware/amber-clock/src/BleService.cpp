#include "BleService.hpp"
#include "Logger.hpp"
#include "AcpProtocol.hpp"
#include "CommandSource.hpp"
#include "Amber.hpp"

#define AMBER_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define AMBER_RX_CHAR_UUID "12345678-1234-5678-1234-56789abcdef1"
#define AMBER_TX_CHAR_UUID "12345678-1234-5678-1234-56789abcdef2"

namespace amber {

BleService::BleService() {}

bool BleService::begin() {
    LOG_INFO("BLE EVENT: Initializing");
    _activeCommand[0] = '\0';
    BLEDevice::init("Amber Clock");
    _pServer = BLEDevice::createServer();
    if (_pServer == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create BLE server");
        return false;
    }
    _pServer->setCallbacks(this);

    // Create the custom Amber Service
    _pService = _pServer->createService(AMBER_SERVICE_UUID);
    if (_pService == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create Amber Service");
        return false;
    }
    LOG_INFO("BLE EVENT: Service created");

    // Create writable RX characteristic
    _pRxChar = _pService->createCharacteristic(
        AMBER_RX_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    if (_pRxChar == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create RX characteristic");
        return false;
    }
    _pRxChar->setCallbacks(this);
    LOG_INFO("BLE EVENT: RX ready");

    // Create readable/notify TX characteristic
    _pTxChar = _pService->createCharacteristic(
        AMBER_TX_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
    );
    if (_pTxChar == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create TX characteristic");
        return false;
    }
    _pTxChar->addDescriptor(new BLE2902());
    LOG_INFO("BLE EVENT: TX ready");

    _pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID(AMBER_SERVICE_UUID));
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    
    BLEDevice::startAdvertising();
    LOG_INFO("BLE EVENT: Advertising started");
    return true;
}

void BleService::update() {
    uint32_t currentMs = millis();

    if (!_connected && _wasConnected) {
        delay(500); 
        _pServer->startAdvertising();
        LOG_INFO("BLE EVENT: Advertising restarted");
        _wasConnected = false;
        
        // Reset sub queue states upon complete disconnect
        _queueHead = 0;
        _queueTail = 0;
        _queueSize = 0;
        _subscribed = false;
        _pendingCmd = false;
        _pendingLen = 0;
        _burstCount = 0;
        _isDraining = false;
    }
    if (_connected && !_wasConnected) {
        _wasConnected = true;
    }

    // Process a pending command outside callback contexts
    if (_pendingCmd) {
        _pendingCmd = false;
        
        // Setup initial drain metrics timers
        _drainStartMs = millis();
        _isDraining = true;
        _chunksCountThisResponse = 0;
        _maxQueueDepth = 0; // Reset response-specific peak depth check on command init

        Amber::processCommand(CommandSource::BLE, (const uint8_t*)_pendingRx, _pendingLen);
        _pendingLen = 0;
    }

    // Process our non-blocking fixed-size static TX Queue gradually to preserve client memory
    if (_connected && _queueSize > 0) {
        // Evaluate conservative burst pacing config parameters:
        // - default send interval spacing: 50ms
        // - max consecutive packets before cooldown: 8 chunks
        // - burst cooldown: 100ms
        uint32_t currentSpacing = (_burstCount >= 8) ? 100 : 50;

        if (currentMs - _lastTxMs >= currentSpacing) {
            _lastTxMs = currentMs;

            // Reset bursts after cooldown completes
            if (_burstCount >= 8) {
                _burstCount = 0;
            }

            // Verify if client subscribed to TX notifications
            BLEDescriptor* desc = _pTxChar->getDescriptorByUUID("2902");
            _subscribed = (desc != nullptr);

            if (_subscribed) {
                _pTxChar->setValue((uint8_t*)_txQueue[_queueHead], strlen(_txQueue[_queueHead]));
                
                // execute notify() (Note: standard Arduino ble notify() yields void, return limits listed in specs)
                _pTxChar->notify();

                _queueHead = (_queueHead + 1) % QueueCapacity;
                _queueSize--;
                _burstCount++;
                _chunksCountThisResponse++;

                // Log summaries once complete transfer finishes
                if (_queueSize == 0 && _isDraining) {
                    _isDraining = false;
                    _burstCount = 0;
                    uint32_t elapsed = millis() - _drainStartMs;
                    
                    char sumBuf[160];
                    snprintf(sumBuf, sizeof(sumBuf), 
                             "BLE TX RESPONSE | COMMAND=%s | CHUNKS=%u | MAX_QUEUE_DEPTH=%u | GLOBAL_MAX_QUEUE_DEPTH=%u | DRAIN_MS=%lu | OVERFLOWS=%u | NOTIFY_FAILURES=%u",
                             _activeCommand[0] != '\0' ? _activeCommand : "UNKNOWN",
                             (unsigned int)_chunksCountThisResponse,
                             (unsigned int)_maxQueueDepth,
                             (unsigned int)_globalMaxQueueDepth,
                             elapsed,
                             (unsigned int)_overflows,
                             (unsigned int)_notifyFailures);
                    LOG_INFO(sumBuf);

                    // clear response-specific parameters
                    _maxQueueDepth = 0;
                    _overflows = 0;
                }
            }
        }
    }
}

bool BleService::isConnected() const {
    return _connected;
}

bool BleService::isSubscribed() const {
    return _subscribed;
}

void BleService::onConnect(BLEServer* pServer) {
    _connected = true;
    _queueHead = 0;
    _queueTail = 0;
    _queueSize = 0;
    _subscribed = false;
    _burstCount = 0;
    _isDraining = false;
    _maxQueueDepth = 0; // Reset response peak tracking upon reconnection
    LOG_INFO("BLE EVENT: Connected");
}

void BleService::onDisconnect(BLEServer* pServer) {
    _connected = false;
    _subscribed = false;
    _burstCount = 0;
    _isDraining = false;
    LOG_INFO("BLE EVENT: Disconnected");
}

void BleService::setActiveCommand(const char* cmd) {
    strncpy(_activeCommand, cmd, sizeof(_activeCommand) - 1);
    _activeCommand[sizeof(_activeCommand) - 1] = '\0';
}

void BleService::enqueueChunk(const char* chunk) {
    if (_queueSize >= QueueCapacity) {
        _overflows++;
        char overBuf[128];
        snprintf(overBuf, sizeof(overBuf), 
                 "BLE TX QUEUE OVERFLOW DEPTH=%u COMMAND=%s", 
                 (unsigned int)_queueSize, 
                 _activeCommand[0] != '\0' ? _activeCommand : "UNKNOWN");
        LOG_INFO(overBuf);
        return; // Abort clean to preserve data integrity and prevent overwrite corruptions
    }
    size_t len = strlen(chunk);
    if (len > ChunkSize) len = ChunkSize;
    
    memcpy(_txQueue[_queueTail], chunk, len);
    _txQueue[_queueTail][len] = '\0';

    _queueTail = (_queueTail + 1) % QueueCapacity;
    _queueSize++;

    // Track response-specific peak capacity
    if (_queueSize > _maxQueueDepth) {
        _maxQueueDepth = _queueSize;
    }

    // Track dynamic lifetime high-water record reached since boot
    if (_queueSize > _globalMaxQueueDepth) {
        _globalMaxQueueDepth = _queueSize;
        char depthBuf[64];
        snprintf(depthBuf, sizeof(depthBuf), "GLOBAL_MAX_QUEUE_DEPTH=%u", (unsigned int)_globalMaxQueueDepth);
        LOG_INFO(depthBuf);
    }
}

void BleService::enqueueResponseLine(const char* line) {
    size_t totalLen = strlen(line);
    size_t offset = 0;

    // Split messages exceeding safe ChunkSize parameter safely into multiple 20-byte notifications
    while (offset < totalLen) {
        char chunk[ChunkSize + 1];
        size_t copyBytes = totalLen - offset;
        if (copyBytes > ChunkSize) copyBytes = ChunkSize;

        memcpy(chunk, &line[offset], copyBytes);
        chunk[copyBytes] = '\0';

        enqueueChunk(chunk);
        offset += copyBytes;
    }
}

void BleService::onWrite(BLECharacteristic* pCharacteristic) {
    LOG_INFO("BLE RX CALLBACK ENTER");
    
    std::string value = pCharacteristic->getValue();
    size_t len = value.length();
    
    if (len > 0) {
        if (len > 64) {
            LOG_INFO("BLE RX QUEUED LEN=OVERSIZED_REJECTED");
            LOG_INFO("BLE RX CALLBACK EXIT");
            return;
        }

        // Copy bytes into lightweight fixed-size mailbox
        memcpy(_pendingRx, value.data(), len);
        _pendingRx[len] = '\0';
        _pendingLen = len;
        _pendingCmd = true;

        char logBuf[64];
        snprintf(logBuf, sizeof(logBuf), "BLE RX QUEUED LEN=%d", (int)len);
        LOG_INFO(logBuf);
    }
    
    LOG_INFO("BLE RX CALLBACK EXIT");
}

} // namespace amber
