#pragma once
#include <stdint.h>
#include <stddef.h>

namespace amber {

enum class AcpCommandType {
    Unknown,
    Ping,
    Version,
    GetTime,
    SetTime,
    Status,
    GetBrightness,
    SetBrightness,
    Help,
    Info,
    Health
};

enum class HealthSubCommand {
    Query,
    On,
    Off,
    SetInterval
};

struct AcpTimeValue {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct AcpCommand {
    AcpCommandType type = AcpCommandType::Unknown;
    AcpTimeValue time = {0, 0, 0};
    uint8_t brightnessPercent = 0;
    char helpArg[16] = {0};
    char infoArg[16] = {0};
    HealthSubCommand healthSub = HealthSubCommand::Query;
    uint32_t healthIntervalMs = 1000;
};

enum class AcpParseError {
    None,
    Empty,
    TooLong,
    UnknownCommand,
    MissingArgument,
    InvalidFormat,
    OutOfRange,
    InvalidSyntax
};

struct AcpParseResult {
    bool success = false;
    AcpCommand command;
    AcpParseError error = AcpParseError::None;
};

struct RegisterItem {
    const char* name;
    AcpCommandType type;
    const char* category;
    const char* usage;
    const char* description;
    const char* status;
    size_t SinceVersion;
    uint8_t argCount;
    const char* argDescription;
    const char* argFormat;
};

class CommandRegistry {
public:
    static constexpr size_t TotalCommands = 10; // 10 commands: PING, VERSION, HELP, GETTIME, SETTIME, STATUS, GETBRIGHTNESS, BRIGHTNESS, INFO, HEALTH
    static const RegisterItem Items[TotalCommands];

    static const RegisterItem* findByName(const char* name);
    static const RegisterItem* findByType(AcpCommandType type);
};

class AcpProtocol {
public:
    static AcpParseResult parse(const uint8_t* data, size_t length);
};

} // namespace amber
