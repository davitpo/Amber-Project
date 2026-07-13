#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

namespace amber {

class AcpResponseWriter {
public:
    virtual void writeLine(const char* line) = 0;
    virtual void endResponse() = 0;
    virtual ~AcpResponseWriter() = default;
};

class BleService : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    BleService();
    bool begin();
    void update();
    bool isConnected() const;
    bool isSubscribed() const;

    // Queue text for deferred multi-notification chunking
    void enqueueResponseLine(const char* line);

    // Track active command for diagnostics tag mapping
    void setActiveCommand(const char* cmd);

private:
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
    void onWrite(BLECharacteristic* pCharacteristic) override;

    BLEServer* _pServer = nullptr;
    ::BLEService* _pService = nullptr;
    BLECharacteristic* _pRxChar = nullptr;
    BLECharacteristic* _pTxChar = nullptr;
    bool _connected = false;
    bool _wasConnected = false;
    bool _subscribed = false;

    // Lightweight Pending RX Mailbox
    char _pendingRx[65];
    size_t _pendingLen = 0;
    bool _pendingCmd = false;

    // Flow stabilized 96-chunk safe dynamic queue bounds
    static constexpr size_t QueueCapacity = 96;
    static constexpr size_t ChunkSize = 20;

    char _txQueue[QueueCapacity][ChunkSize + 1];
    size_t _queueHead = 0;
    size_t _queueTail = 0;
    size_t _queueSize = 0;
    uint32_t _lastTxMs = 0;

    // Burst & Backpressure Tracking
    uint32_t _burstCount = 0;
    uint32_t _drainStartMs = 0;
    bool _isDraining = false;

    // Diagnostics Indicators
    size_t _maxQueueDepth = 0;
    size_t _globalMaxQueueDepth = 0; // Separate lifetime high-water metric (GLOBAL_MAX_QUEUE_DEPTH)
    uint32_t _overflows = 0;
    uint32_t _notifyFailures = 0;
    uint32_t _chunksCountThisResponse = 0;
    char _activeCommand[32] = {0};

    void enqueueChunk(const char* chunk);
};

} // namespace amber
