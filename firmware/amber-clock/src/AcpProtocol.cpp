#include "AcpProtocol.hpp"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

namespace amber {

AcpParseResult AcpProtocol::parse(const uint8_t* data, size_t length) {
    AcpParseResult result;
    result.success = false;
    result.command.type = AcpCommandType::Unknown;
    result.error = AcpParseError::None;

    if (length == 0) {
        result.error = AcpParseError::Empty;
        return result;
    }
    if (length > 64) {
        result.error = AcpParseError::TooLong;
        return result;
    }

    // Work on a mutable stack copy and strip/trim spaces
    char buffer[65];
    size_t start = 0;
    while (start < length && isspace(data[start])) {
        start++;
    }
    size_t end = length;
    while (end > start && isspace(data[end - 1])) {
        end--;
    }
    size_t cleanLen = end - start;
    if (cleanLen == 0) {
        result.error = AcpParseError::Empty;
        return result;
    }
    memcpy(buffer, &data[start], cleanLen);
    buffer[cleanLen] = '\0';

    // Normalize to upper case
    for (size_t i = 0; i < cleanLen; i++) {
        buffer[i] = toupper((unsigned char)buffer[i]);
    }

    // Reject '=' formatting to migrate to whitespace separation keys
    if (strchr(buffer, '=') != nullptr) {
        result.error = AcpParseError::InvalidSyntax;
        return result;
    }

    // Split based on whitespace / tab tokens
    char* cmdKey = buffer;
    char* argVal = nullptr;
    
    // Find first whitespace separation boundary
    size_t cursor = 0;
    while (cursor < cleanLen && !isspace((unsigned char)buffer[cursor])) {
        cursor++;
    }
    if (cursor < cleanLen) {
        buffer[cursor] = '\0';
        argVal = &buffer[cursor + 1];
        // Strip out subsequent leading whitespaces inside argument values
        while (*argVal != '\0' && isspace((unsigned char)*argVal)) {
            argVal++;
        }
        if (*argVal == '\0') {
            argVal = nullptr;
        }
    }

    // Query Command Registry
    const RegisterItem* item = CommandRegistry::findByName(cmdKey);
    if (item == nullptr) {
        result.error = AcpParseError::UnknownCommand;
        return result;
    }

    if (item->argCount == 0 && argVal != nullptr) {
        result.error = AcpParseError::InvalidFormat;
        return result;
    }

    if (item->type == AcpCommandType::SetTime) {
        if (argVal == nullptr) {
            result.error = AcpParseError::MissingArgument;
            return result;
        }
        if (strlen(argVal) != 8 || argVal[2] != ':' || argVal[5] != ':') {
            result.error = AcpParseError::InvalidFormat;
            return result;
        }
        for (int i = 0; i < 8; i++) {
            if (i != 2 && i != 5 && !isdigit((unsigned char)argVal[i])) {
                result.error = AcpParseError::InvalidFormat;
                return result;
            }
        }
        int h = atoi(argVal);
        int m = atoi(argVal + 3);
        int s = atoi(argVal + 6);
        if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) {
            result.error = AcpParseError::OutOfRange;
            return result;
        }
        result.success = true;
        result.command.type = AcpCommandType::SetTime;
        result.command.time.hour = (uint8_t)h;
        result.command.time.minute = (uint8_t)m;
        result.command.time.second = (uint8_t)s;
        return result;
    }

    if (item->type == AcpCommandType::SetBrightness) {
        if (argVal == nullptr) {
            result.error = AcpParseError::MissingArgument;
            return result;
        }
        for (size_t i = 0; i < strlen(argVal); i++) {
            if (!isdigit((unsigned char)argVal[i])) {
                result.error = AcpParseError::InvalidFormat;
                return result;
            }
        }
        int bright = atoi(argVal);
        if (bright < 0 || bright > 100) {
            result.error = AcpParseError::OutOfRange;
            return result;
        }
        result.success = true;
        result.command.type = AcpCommandType::SetBrightness;
        result.command.brightnessPercent = (uint8_t)bright;
        return result;
    }

    if (item->type == AcpCommandType::Help) {
        result.success = true;
        result.command.type = AcpCommandType::Help;
        if (argVal != nullptr) {
            strncpy(result.command.helpArg, argVal, sizeof(result.command.helpArg) - 1);
        } else {
            result.command.helpArg[0] = '\0';
        }
        return result;
    }

    if (item->type == AcpCommandType::Info) {
        result.success = true;
        result.command.type = AcpCommandType::Info;
        if (argVal != nullptr) {
            strncpy(result.command.infoArg, argVal, sizeof(result.command.infoArg) - 1);
        } else {
            result.command.infoArg[0] = '\0';
        }
        return result;
    }

    if (item->type == AcpCommandType::Health) {
        result.success = true;
        result.command.type = AcpCommandType::Health;
        
        if (argVal == nullptr || strlen(argVal) == 0) {
            result.command.healthSub = HealthSubCommand::Query;
            return result;
        }

        if (strcmp(argVal, "ON") == 0) {
            result.command.healthSub = HealthSubCommand::On;
            return result;
        }
        if (strcmp(argVal, "OFF") == 0) {
            result.command.healthSub = HealthSubCommand::Off;
            return result;
        }

        if (strncmp(argVal, "INTERVAL", 8) == 0) {
            char* valStr = argVal + 8;
            while (*valStr != '\0' && isspace((unsigned char)*valStr)) {
                valStr++;
            }
            if (*valStr == '\0') {
                result.error = AcpParseError::MissingArgument;
                return result;
            }
            for (size_t i = 0; i < strlen(valStr); i++) {
                if (!isdigit((unsigned char)valStr[i])) {
                    result.error = AcpParseError::InvalidFormat;
                    return result;
                }
            }
            long interval = atol(valStr);
            if (interval < 250 || interval > 60000) {
                result.error = AcpParseError::OutOfRange;
                return result;
            }
            result.command.healthSub = HealthSubCommand::SetInterval;
            result.command.healthIntervalMs = (uint32_t)interval;
            return result;
        }

        result.error = AcpParseError::InvalidFormat;
        return result;
    }

    // Default zero-argument fallback handlers
    result.success = true;
    result.command.type = item->type;
    return result;
}

} // namespace amber
