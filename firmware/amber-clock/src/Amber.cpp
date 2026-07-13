#include "Amber.hpp"
#include "Logger.hpp"
#include "AcpProtocol.hpp"
#include <Arduino.h>

namespace amber {

Amber* GlobalAmberInstance = nullptr;

void UartResponseWriter::writeLine(const char* line) {
    char prefixedLine[160];
    snprintf(prefixedLine, sizeof(prefixedLine), "ACP(UART) < %s", line);
    LOG_INFO(prefixedLine);
}

void UartResponseWriter::endResponse() {
    char prefixLine[64];
    snprintf(prefixLine, sizeof(prefixLine), "ACP(UART) < END");
    LOG_INFO(prefixLine);
}

void BleResponseWriter::writeLine(const char* line) {
    if (GlobalAmberInstance != nullptr) {
        // Log debug trace local console prefix
        char prefixedLine[160];
        snprintf(prefixedLine, sizeof(prefixedLine), "ACP(BLE) < %s", line);
        LOG_INFO(prefixedLine);

        // Enqueue response to safe TX Queue
        GlobalAmberInstance->sendAcpResponse(CommandSource::BLE, line);
    }
}

void BleResponseWriter::endResponse() {
    if (GlobalAmberInstance != nullptr) {
        char prefixLine[164];
        snprintf(prefixLine, sizeof(prefixLine), "ACP(BLE) < END");
        LOG_INFO(prefixLine);

        GlobalAmberInstance->sendAcpResponse(CommandSource::BLE, "END");
    }
}

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

void Amber::sendAcpResponse(CommandSource source, const char* msg) {
    if (source == CommandSource::BLE && GlobalAmberInstance != nullptr) {
        GlobalAmberInstance->_ble.enqueueResponseLine(msg);
    }
}

static void sendHelpList(AcpResponseWriter& writer) {
    writer.writeLine("ACP Version 1 Help");
    const char* categories[] = {"System", "Clock", "Display", "Diagnostics"};
    for (size_t c = 0; c < 4; c++) {
        char catHeader[32];
        snprintf(catHeader, sizeof(catHeader), "[%s]", categories[c]);
        writer.writeLine(catHeader);
        for (size_t i = 0; i < CommandRegistry::TotalCommands; i++) {
            auto& item = CommandRegistry::Items[i];
            if (item.name != nullptr && strcmp(item.category, categories[c]) == 0) {
                writer.writeLine(item.name);
            }
        }
    }
    writer.endResponse();
}

static void printCommandInfo(AcpResponseWriter& writer, const RegisterItem& item) {
    char infoBuf[128];
    snprintf(infoBuf, sizeof(infoBuf), "COMMAND=%s", item.name);
    writer.writeLine(infoBuf);
    snprintf(infoBuf, sizeof(infoBuf), "CATEGORY=%s", item.category);
    writer.writeLine(infoBuf);
    snprintf(infoBuf, sizeof(infoBuf), "STATUS=%s", item.status);
    writer.writeLine(infoBuf);
    snprintf(infoBuf, sizeof(infoBuf), "ARGS=%d", item.argCount);
    writer.writeLine(infoBuf);
    if (item.argCount > 0) {
        snprintf(infoBuf, sizeof(infoBuf), "ARG1=%s", item.argDescription);
        writer.writeLine(infoBuf);
        snprintf(infoBuf, sizeof(infoBuf), "FORMAT=%s", item.argFormat);
        writer.writeLine(infoBuf);
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

    // Command trace instrument print
    char commandTrace[128];
    snprintf(commandTrace, sizeof(commandTrace), "ACP COMMAND=%s", valCopy);
    LOG_INFO(commandTrace);

    // Associate current parsing key reference inside BleService diagnostics trackers
    if (source == CommandSource::BLE) {
        GlobalAmberInstance->_ble.setActiveCommand(valCopy);
    }

    LOG_INFO("ACP EXECUTE BEGIN");
    uint32_t buildStartMs = millis();

    // Polymorphically allocate the appropriate writer context based on active transport source
    UartResponseWriter uartWriter;
    BleResponseWriter bleWriter;
    AcpResponseWriter* pWriter = nullptr;
    if (source == CommandSource::UART) {
        pWriter = &uartWriter;
    } else {
        pWriter = &bleWriter;
    }

    AcpParseResult result = AcpProtocol::parse(data, length);
    if (result.success) {
        char logMsg[128];
        switch (result.command.type) {
            case AcpCommandType::Ping:
                pWriter->writeLine("PONG");
                break;
            case AcpCommandType::Version:
                pWriter->writeLine("VERSION=0.5.0");
                break;
            case AcpCommandType::GetTime: {
                ClockTime ct = GlobalAmberInstance->getLocalTime();
                char timeBuf[64];
                snprintf(timeBuf, sizeof(timeBuf), "TIME=%02d:%02d:%02d", ct.hour, ct.minute, ct.second);
                pWriter->writeLine(timeBuf);
                break;
            }
            case AcpCommandType::Status: {
                char statBuf[128];
                snprintf(statBuf, sizeof(statBuf), "TIME=%02d:%02d:%02d", 
                         GlobalAmberInstance->getClockHour(), GlobalAmberInstance->getClockMinute(), GlobalAmberInstance->getClockSecond());
                pWriter->writeLine(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "RENDER_MODE=%s", GlobalAmberInstance->getRenderModeStr());
                pWriter->writeLine(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "BLE=%s", GlobalAmberInstance->isBleConnected() ? "CONNECTED" : "DISCONNECTED");
                pWriter->writeLine(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "HEAP_FREE=%u", GlobalAmberInstance->getFreeHeap());
                pWriter->writeLine(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "HEAP_MIN=%u", GlobalAmberInstance->getMinFreeHeap());
                pWriter->writeLine(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "FPS=%u", GlobalAmberInstance->getFps());
                pWriter->writeLine(statBuf);
                
                snprintf(statBuf, sizeof(statBuf), "UPTIME_MS=%lu", millis());
                pWriter->writeLine(statBuf);
                
                pWriter->endResponse();
                break;
            }
            case AcpCommandType::GetBrightness: {
                char brightBuf[32];
                snprintf(brightBuf, sizeof(brightBuf), "BRIGHTNESS=%u", GlobalAmberInstance->getBrightnessPercent());
                pWriter->writeLine(brightBuf);
                break;
            }
            case AcpCommandType::SetTime:
                GlobalAmberInstance->setLocalTime(result.command.time.hour, result.command.time.minute, result.command.time.second);
                pWriter->writeLine("OK");
                break;
            case AcpCommandType::SetBrightness:
                GlobalAmberInstance->setBrightnessPercent(result.command.brightnessPercent);
                pWriter->writeLine("OK");
                break;
            case AcpCommandType::Help:
                if (result.command.helpArg[0] != '\0') {
                    const RegisterItem* spec = CommandRegistry::findByName(result.command.helpArg);
                    if (spec != nullptr) {
                        pWriter->writeLine("--------------------------------");
                        snprintf(logMsg, sizeof(logMsg), "Command     : %s", spec->name);
                        pWriter->writeLine(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Category    : %s", spec->category);
                        pWriter->writeLine(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Usage       : %s", spec->usage);
                        pWriter->writeLine(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Description : %s", spec->description);
                        pWriter->writeLine(logMsg);
                        snprintf(logMsg, sizeof(logMsg), "Status      : %s", spec->status);
                        pWriter->writeLine(logMsg);
                        pWriter->writeLine("--------------------------------");
                        pWriter->endResponse();
                    } else {
                        pWriter->writeLine("ERROR=UNKNOWN_COMMAND");
                    }
                } else {
                    sendHelpList(*pWriter);
                }
                break;
            case AcpCommandType::Info:
                if (result.command.infoArg[0] != '\0') {
                    if (strcmp(result.command.infoArg, "ALL") == 0) {
                        for (size_t i = 0; i < CommandRegistry::TotalCommands; i++) {
                            printCommandInfo(*pWriter, CommandRegistry::Items[i]);
                            pWriter->writeLine("---");
                        }
                        pWriter->endResponse();
                    } else {
                        const RegisterItem* spec = CommandRegistry::findByName(result.command.infoArg);
                        if (spec != nullptr) {
                            printCommandInfo(*pWriter, *spec);
                            pWriter->endResponse();
                        } else {
                            pWriter->writeLine("ERROR=UNKNOWN_COMMAND");
                        }
                    }
                } else {
                    pWriter->writeLine("ERROR=MISSING_ARGUMENT");
                }
                break;
            case AcpCommandType::Health:
                if (result.command.healthSub == HealthSubCommand::Query) {
                    char healthBuf[128];
                    snprintf(healthBuf, sizeof(healthBuf), "HEALTH_ENABLED=%s", 
                             GlobalAmberInstance->_healthMonitor.isEnabled() ? "TRUE" : "FALSE");
                    pWriter->writeLine(healthBuf);
                    snprintf(healthBuf, sizeof(healthBuf), "HEALTH_INTERVAL_MS=%u", 
                             GlobalAmberInstance->_healthMonitor.getInterval());
                    pWriter->writeLine(healthBuf);
                    pWriter->endResponse();
                } else if (result.command.healthSub == HealthSubCommand::On) {
                    GlobalAmberInstance->_healthMonitor.setEnabled(true);
                    pWriter->writeLine("HEALTH_ENABLED=TRUE");
                } else if (result.command.healthSub == HealthSubCommand::Off) {
                    GlobalAmberInstance->_healthMonitor.setEnabled(false);
                    pWriter->writeLine("HEALTH_ENABLED=FALSE");
                } else if (result.command.healthSub == HealthSubCommand::SetInterval) {
                    GlobalAmberInstance->_healthMonitor.setInterval(result.command.healthIntervalMs);
                    char intervalBuf[64];
                    snprintf(intervalBuf, sizeof(intervalBuf), "HEALTH_INTERVAL_MS=%u", result.command.healthIntervalMs);
                    pWriter->writeLine(intervalBuf);
                }
                break;
            default:
                pWriter->writeLine("ERROR=UNKNOWN_COMMAND");
                break;
        }
    } else {
        switch (result.error) {
            case AcpParseError::Empty:
                break;
            case AcpParseError::TooLong:
                pWriter->writeLine("ERROR=TOO_LONG");
                break;
            case AcpParseError::UnknownCommand:
                pWriter->writeLine("ERROR=UNKNOWN_COMMAND");
                break;
            case AcpParseError::MissingArgument:
                pWriter->writeLine("ERROR=MISSING_ARGUMENT");
                break;
            case AcpParseError::InvalidFormat:
                pWriter->writeLine("ERROR=INVALID_FORMAT");
                break;
            case AcpParseError::OutOfRange:
                pWriter->writeLine("ERROR=OUT_OF_RANGE");
                break;
            case AcpParseError::InvalidSyntax:
                pWriter->writeLine("ERROR=INVALID_SYNTAX");
                break;
            default:
                pWriter->writeLine("ERROR=UNKNOWN_COMMAND");
                break;
        }
    }

    uint32_t executeDurationMs = millis() - buildStartMs;
    char executeTimeBuf[64];
    snprintf(executeTimeBuf, sizeof(executeTimeBuf), "ACP_EXECUTE_MS=%lu", executeDurationMs);
    LOG_INFO(executeTimeBuf);

    LOG_INFO("ACP EXECUTE END");
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
