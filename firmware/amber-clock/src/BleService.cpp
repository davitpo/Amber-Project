#include "BleService.hpp"
#include "Logger.hpp"
#include "AcpProtocol.hpp"
#include "CommandSource.hpp"
#include "Amber.hpp"

#define AMBER_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define AMBER_RX_CHAR_UUID "12345678-1234-5678-1234-56789abcdef1"

namespace amber {

BleService::BleService() {}

bool BleService::begin() {
    LOG_INFO("BLE EVENT: Initializing");
    BLEDevice::init("Amber Clock");
    _pServer = BLEDevice::createServer();
    if (_pServer == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create BLE server");
        return false;
    }
    _pServer->setCallbacks(this);

    // Create the custom Amber Service
    ::BLEService* pService = _pServer->createService(AMBER_SERVICE_UUID);
    if (pService == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create Amber Service");
        return false;
    }
    LOG_INFO("BLE EVENT: Service created");

    // Create writable RX characteristic
    _pRxChar = pService->createCharacteristic(
        AMBER_RX_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    if (_pRxChar == nullptr) {
        LOG_INFO("BLE ERROR: Failed to create RX characteristic");
        return false;
    }
    _pRxChar->setCallbacks(this);
    LOG_INFO("BLE EVENT: RX ready");

    pService->start();

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
    if (!_connected && _wasConnected) {
        delay(500); 
        _pServer->startAdvertising();
        LOG_INFO("BLE EVENT: Advertising restarted");
        _wasConnected = false;
    }
    if (_connected && !_wasConnected) {
        _wasConnected = true;
    }
}

bool BleService::isConnected() const {
    return _connected;
}

void BleService::onConnect(BLEServer* pServer) {
    _connected = true;
    LOG_INFO("BLE EVENT: Connected");
}

void BleService::onDisconnect(BLEServer* pServer) {
    _connected = false;
    LOG_INFO("BLE EVENT: Disconnected");
}

static void logGroupedHelp() {
    LOG_INFO("ACP(BLE): HELP ->");
    LOG_INFO("ACP Version 1");
    
    const char* categories[] = {"System", "Clock", "Display", "Diagnostics"};
    for (size_t c = 0; c < 4; c++) {
        char catHeader[32];
        snprintf(catHeader, sizeof(catHeader), "[%s]", categories[c]);
        LOG_INFO(catHeader);
        for (size_t i = 0; i < CommandRegistry::TotalCommands; i++) {
            auto& item = CommandRegistry::Items[i];
            if (item.name != nullptr && strcmp(item.category, categories[c]) == 0) {
                LOG_INFO(item.name);
            }
        }
    }
}

void BleService::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
        // Delegate writing events to shared ACP multi-transports processCommand router
        Amber::processCommand(CommandSource::BLE, (const uint8_t*)value.data(), value.length());
    }
}

} // namespace amber
