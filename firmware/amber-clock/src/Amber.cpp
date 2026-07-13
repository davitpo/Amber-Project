#include "Amber.hpp"
#include "Logger.hpp"
#include "AcpProtocol.hpp"
#include <Arduino.h>

namespace amber {

Amber* GlobalAmberInstance = nullptr;

void Amber::begin() {
    GlobalAmberInstance = this;
    
    // Explicit Memory Checkpoints
    uint32_t heapBoot = ESP.getFreeHeap();
    
    _clock.setTime(0, 36, 0);
    _lastTickMs = millis();
    _lastRenderMs = millis();
    _lastFpsMetricsMs = millis();

    LOG_INFO("Clock initialized");

    _display.begin();
    
    uint32_t heapAfterDisplay = ESP.getFreeHeap();
    uint32_t heapAfterSprite = ESP.getFreeHeap(); // Target handles this during .begin()

    uint32_t heapBeforeBle = ESP.getFreeHeap();
    LOG_INFO("BLE EVENT: Initializing");
    bool bleSuccess = _ble.begin();
    uint32_t heapAfterBle = ESP.getFreeHeap();
    int32_t bleCost = heapBeforeBle - heapAfterBle;

    // Log Explicit term calculations
    char memBuf[128];
    snprintf(memBuf, sizeof(memBuf), "MEM BOOT=%u", heapBoot);
    LOG_INFO(memBuf);
    snprintf(memBuf, sizeof(memBuf), "MEM DISPLAY=%u", heapAfterDisplay);
    LOG_INFO(memBuf);
    snprintf(memBuf, sizeof(memBuf), "MEM SPRITE=%u", heapAfterSprite);
    LOG_INFO(memBuf);
    snprintf(memBuf, sizeof(memBuf), "MEM BLE_BEFORE=%u", heapBeforeBle);
    LOG_INFO(memBuf);
    snprintf(memBuf, sizeof(memBuf), "MEM BLE_AFTER=%u", heapAfterBle);
    LOG_INFO(memBuf);
    snprintf(memBuf, sizeof(memBuf), "MEM BLE_COST=%d", bleCost);
    LOG_INFO(memBuf);
    
    renderFrame(0.0f);

    LOG_INFO("Amber initialized");
}

void Amber::update() {
    updateClock();
    calculateFps();
    _ble.update();
    _serialTransport.update();
    processEvents();
    runPeriodicTelemetry();
}

void Amber::updateClock() {
    uint32_t currentMs = millis();
    
    if (currentMs - _lastTickMs >= TickIntervalMs) {
        _lastTickMs += currentMs - _lastTickMs; // align precisely
        _clock.tick();
    }

    uint32_t interval = (_display.renderMode() == RenderMode::FullSprite) ? RenderIntervalMsFull : RenderIntervalMsDirect;
    if (currentMs - _lastRenderMs >= interval) {
        _lastRenderMs = currentMs;
        
        float fractionalSecond = 0.0f;
        if (_display.renderMode() == RenderMode::FullSprite) {
            uint32_t elapsedMs = currentMs - _lastTickMs;
            if (elapsedMs > TickIntervalMs) {
                elapsedMs = TickIntervalMs;
            }
            fractionalSecond = static_cast<float>(elapsedMs) / 1000.0f;
            if (fractionalSecond > 0.999f) {
                fractionalSecond = 0.999f;
            }
        }
        
        renderFrame(fractionalSecond);
        _frameCount++;
    }
}

void Amber::calculateFps() {
    uint32_t currentMs = millis();
    uint32_t elapsedMs = currentMs - _lastFpsMetricsMs;
    // Calculate FPS every 1000ms internally regardless of telemetry output state
    if (elapsedMs >= 1000) {
        _fps = (_frameCount * 1000) / elapsedMs;
        _frameCount = 0;
        _lastFpsMetricsMs = currentMs;
    }
}

void Amber::renderFrame(float fractionalSecond) {
    auto& canvas = _display.target();
    renderStaticLayer(canvas);
    renderDynamicLayer(canvas, fractionalSecond);
    _display.pushFrame();
}

void Amber::renderStaticLayer(LovyanGFX& target) {
    _clockFace.draw(target);
}

void Amber::renderDynamicLayer(LovyanGFX& target, float fractionalSecond) {
    _clockHands.draw(target, _clock.getAngles(fractionalSecond));
}

static void sendResponse(CommandSource source, const char* msg) {
    char prefixedResponse[160];
    const char* transportTag = (source == CommandSource::UART) ? "ACP(UART)" : "ACP(BLE)";
    
    // Add uniform responsive trace markers
    snprintf(prefixedResponse, sizeof(prefixedResponse), "%s < %s", transportTag, msg);
    LOG_INFO(prefixedResponse);
}

static void sendHelpList(CommandSource source) {
    const char* transportTag = (source == CommandSource::UART) ? "ACP(UART)" : "ACP(BLE)";
    
    char prefixLine[64];
    snprintf(prefixLine, sizeof(prefixLine), "%s <", transportTag);
    LOG_INFO(prefixLine);
    
    LOG_INFO("ACP Version 1 Help");
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
    LOG_INFO("END");
}

static void printCommandInfo(CommandSource source, const RegisterItem& item) {
    char infoBuf[128];
    snprintf(infoBuf, sizeof(infoBuf), "COMMAND=%s", item.name);
    sendResponse(source, infoBuf);
    snprintf(infoBuf, sizeof(infoBuf), "CATEGORY=%s", item.category);
    sendResponse(source, infoBuf);
    snprintf(infoBuf, sizeof(infoBuf), "STATUS=%s", item.status);
    sendResponse(source, infoBuf);
    snprintf(infoBuf, sizeof(infoBuf), "ARGS=%d", item.argCount);
    sendResponse(source, infoBuf);
    if (item.argCount > 0) {
        snprintf(infoBuf, sizeof(infoBuf), "ARG1=%s", item.argDescription);
        sendResponse(source, infoBuf);
        snprintf(infoBuf, sizeof(infoBuf), "FORMAT=%s", item.argFormat);
        sendResponse(source, infoBuf);
    }
}

void Amber::processCommand(CommandSource source, const uint8_t* data, size_t length) {
    if (GlobalAmberInstance == nullptr) {
        return;
    }

    const char* srcTag = (source == CommandSource::UART) ? "ACP(UART)" : "ACP(BLE)";
    
    char valCopy[65];
    size_t copyLen = length < 64 ? length : 64;
    memcpy(valCopy, data, copyLen);
    valCopy[copyLen] = '\0';
    
    char debugTrace[128];
    snprintf(debugTrace, sizeof(debugTrace), "%s > %s", srcTag, valCopy);
    LOG_INFO(debugTrace);

    AcpParseResult result = AcpProtocol::parse(data, length);
    if (result.success) {
        char logMsg[128];
        switch (result.command.type) {
            case AcpCommandType::Ping:
                sendResponse(source, "PONG");
                break;
            case AcpCommandType::Version:
                sendResponse(source, "VERSION=0.5.0");
                break;
            case AcpCommandType::GetTime: {
                ClockTime ct = GlobalAmberInstance->getLocalTime();
                char timeBuf[64];
                snprintf(timeBuf, sizeof(timeBuf), "TIME=%02d:%02d:%02d", ct.hour, ct.minute, ct.second);
                sendResponse(source, timeBuf);
                break;
            }
            case AcpCommandType::Status: {
                // Return multi-line responses cleanly prefixing lines and terminating in END
                char prefixLine[64];
                snprintf(prefixLine, sizeof(prefixLine), "%s <", srcTag);
                LOG_INFO(prefixLine);

                char statBuf[128];
                snprintf(statBuf, sizeof(statBuf), "TIME=%02d:%02d:%02d", 
                         GlobalAmberInstance->getClockHour(), GlobalAmberInstance->getClockMinute(), GlobalAmberInstance->getClockSecond());
                LOG_INFO(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "RENDER_MODE=%s", GlobalAmberInstance->getRenderModeStr());
                LOG_INFO(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "BLE=%s", GlobalAmberInstance->isBleConnected() ? "CONNECTED" : "DISCONNECTED");
                LOG_INFO(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "HEAP_FREE=%u", GlobalAmberInstance->getFreeHeap());
                LOG_INFO(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "HEAP_MIN=%u", GlobalAmberInstance->getMinFreeHeap());
                LOG_INFO(statBuf);
                
                // Continuous, active FPS metrics reporting
                snprintf(statBuf, sizeof(statBuf), "FPS=%u", GlobalAmberInstance->getFps());
                LOG_INFO(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "UPTIME_MS=%lu", millis());
                LOG_INFO(statBuf);
                
                LOG_INFO("END");
                break;
            }
            case AcpCommandType::GetBrightness:
                sendResponse(source, "ERROR=NOT_IMPLEMENTED");
                break;
            case AcpCommandType::SetTime:
                snprintf(logMsg, sizeof(logMsg), "ACP(%s) Command SETTIME h=%d m=%d s=%d received.", 
                         (source == CommandSource::UART) ? "UART" : "BLE",
                         result.command.time.hour, result.command.time.minute, result.command.time.second);
                LOG_INFO(logMsg);
                sendResponse(source, "ERROR=NOT_IMPLEMENTED");
                break;
            case AcpCommandType::SetBrightness:
                snprintf(logMsg, sizeof(logMsg), "ACP(%s) Command BRIGHTNESS value=%d received.", 
                         (source == CommandSource::UART) ? "UART" : "BLE",
                         result.command.brightnessPercent);
                LOG_INFO(logMsg);
                sendResponse(source, "ERROR=NOT_IMPLEMENTED");
                break;
            case AcpCommandType::Help:
                if (result.command.helpArg[0] != '\0') {
                    const RegisterItem* spec = CommandRegistry::findByName(result.command.helpArg);
                    if (spec != nullptr) {
                        char prefixLine[64];
                        snprintf(prefixLine, sizeof(prefixLine), "%s <", srcTag);
                        LOG_INFO(prefixLine);

                        LOG_INFO("--------------------------------");
                        snprintf(logMsg, sizeof(logMsg), "Command     : %s", spec->name);
                        LOG_INFO(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Category    : %s", spec->category);
                        LOG_INFO(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Usage       : %s", spec->usage);
                        LOG_INFO(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Description : %s", spec->description);
                        LOG_INFO(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Status      : %s", spec->status);
                        LOG_INFO(logMsg);
                        LOG_INFO("--------------------------------");
                        LOG_INFO("END");
                    } else {
                        sendResponse(source, "ERROR=UNKNOWN_COMMAND");
                    }
                } else {
                    sendHelpList(source);
                }
                break;
            case AcpCommandType::Info:
                if (result.command.infoArg[0] != '\0') {
                    if (strcmp(result.command.infoArg, "ALL") == 0) {
                        char prefixLine[64];
                        snprintf(prefixLine, sizeof(prefixLine), "%s <", srcTag);
                        LOG_INFO(prefixLine);

                        for (size_t i = 0; i < CommandRegistry::TotalCommands; i++) {
                            printCommandInfo(source, CommandRegistry::Items[i]);
                            LOG_INFO("---");
                        }
                        LOG_INFO("END");
                    } else {
                        const RegisterItem* spec = CommandRegistry::findByName(result.command.infoArg);
                        if (spec != nullptr) {
                            char prefixLine[64];
                            snprintf(prefixLine, sizeof(prefixLine), "%s <", srcTag);
                            LOG_INFO(prefixLine);

                            printCommandInfo(source, *spec);
                            LOG_INFO("END");
                        } else {
                            sendResponse(source, "ERROR=UNKNOWN_COMMAND");
                        }
                    }
                } else {
                    sendResponse(source, "ERROR=MISSING_ARGUMENT");
                }
                break;
            case AcpCommandType::Health:
                if (result.command.healthSub == HealthSubCommand::Query) {
                    char prefixLine[64];
                    snprintf(prefixLine, sizeof(prefixLine), "%s <", srcTag);
                    LOG_INFO(prefixLine);

                    char healthBuf[128];
                    snprintf(healthBuf, sizeof(healthBuf), "HEALTH_ENABLED=%s", 
                             GlobalAmberInstance->_healthMonitor.isEnabled() ? "TRUE" : "FALSE");
                    LOG_INFO(healthBuf);
                    snprintf(healthBuf, sizeof(healthBuf), "HEALTH_INTERVAL_MS=%u", 
                             GlobalAmberInstance->_healthMonitor.getInterval());
                    LOG_INFO(healthBuf);
                    LOG_INFO("END");
                } else if (result.command.healthSub == HealthSubCommand::On) {
                    GlobalAmberInstance->_healthMonitor.setEnabled(true);
                    sendResponse(source, "HEALTH_ENABLED=TRUE");
                } else if (result.command.healthSub == HealthSubCommand::Off) {
                    GlobalAmberInstance->_healthMonitor.setEnabled(false);
                    sendResponse(source, "HEALTH_ENABLED=FALSE");
                } else if (result.command.healthSub == HealthSubCommand::SetInterval) {
                    GlobalAmberInstance->_healthMonitor.setInterval(result.command.healthIntervalMs);
                    char intervalBuf[64];
                    snprintf(intervalBuf, sizeof(intervalBuf), "HEALTH_INTERVAL_MS=%u", result.command.healthIntervalMs);
                    sendResponse(source, intervalBuf);
                }
                break;
            default:
                sendResponse(source, "ERROR=UNKNOWN_COMMAND");
                break;
        }
    } else {
        switch (result.error) {
            case AcpParseError::Empty:
                break;
            case AcpParseError::TooLong:
                sendResponse(source, "ERROR=TOO_LONG");
                break;
            case AcpParseError::UnknownCommand:
                sendResponse(source, "ERROR=UNKNOWN_COMMAND");
                break;
            case AcpParseError::MissingArgument:
                sendResponse(source, "ERROR=MISSING_ARGUMENT");
                break;
            case AcpParseError::InvalidFormat:
                sendResponse(source, "ERROR=INVALID_FORMAT");
                break;
            case AcpParseError::OutOfRange:
                sendResponse(source, "ERROR=OUT_OF_RANGE");
                break;
            case AcpParseError::InvalidSyntax:
                sendResponse(source, "ERROR=INVALID_SYNTAX");
                break;
            default:
                sendResponse(source, "ERROR=UNKNOWN_COMMAND");
                break;
        }
    }
}

void Amber::runPeriodicTelemetry() {
    uint32_t currentMs = millis();
    if (_healthMonitor.shouldRun(currentMs)) {
        char periodicBuf[128];
        snprintf(periodicBuf, sizeof(periodicBuf), 
                 "HEALTH TIME=%02d:%02d:%02d MODE=%s BLE=%d HEAP=%u MIN=%u FPS=%u",
                 getClockHour(), getClockMinute(), getClockSecond(),
                 getRenderModeStr(),
                 isBleConnected() ? 1 : 0,
                 getFreeHeap(), getMinFreeHeap(), getFps());
        
        LOG_INFO(periodicBuf);
    }
}

void Amber::processEvents() {
    // Left available for key events / future integrations
}

} // namespace amber
