#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

namespace amber {

class BleService : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    BleService();
    bool begin();
    void update();
    bool isConnected() const;

private:
    // BLEServerCallbacks override methods
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;

    // BLECharacteristicCallbacks override method
    void onWrite(BLECharacteristic* pCharacteristic) override;

    BLEServer* _pServer = nullptr;
    BLEService* _pService = nullptr;
    BLECharacteristic* _pRxChar = nullptr;
    bool _connected = false;
    bool _wasConnected = false;
};

} // namespace amber
