#include "AcpProtocol.hpp"
#include <string.h>

namespace amber {

const RegisterItem CommandRegistry::Items[TotalCommands] = {
    {"PING", AcpCommandType::Ping, "System", "PING", "Communication test", "Implemented", 1, 0, "None", "None"},
    {"VERSION", AcpCommandType::Version, "System", "VERSION", "Get device version", "Implemented", 1, 0, "None", "None"},
    {"HELP", AcpCommandType::Help, "System", "HELP or HELP <command>", "Shows available commands", "Implemented", 1, 1, "Optional target command", "CommandName"},
    {"GETTIME", AcpCommandType::GetTime, "Clock", "GETTIME", "Read current local clock time", "Implemented", 1, 0, "None", "None"},
    {"SETTIME", AcpCommandType::SetTime, "Clock", "SETTIME <hh:mm:ss>", "Set local clock", "Implemented", 1, 1, "Target clock time", "hh:mm:ss"},
    {"STATUS", AcpCommandType::Status, "Diagnostics", "STATUS", "Get diagnostic system reports", "Implemented", 1, 0, "None", "None"},
    {"GETBRIGHTNESS", AcpCommandType::GetBrightness, "Display", "GETBRIGHTNESS", "Get current backlight level", "Implemented", 1, 0, "None", "Percent (0..100)"},
    {"BRIGHTNESS", AcpCommandType::SetBrightness, "Display", "BRIGHTNESS <0..100>", "Set display backlight brightness", "Implemented", 1, 1, "Brightness percentage", "0..100"},
    {"INFO", AcpCommandType::Info, "System", "INFO or INFO <command>", "View low-level command metadata", "Implemented", 1, 1, "Optional query command or ALL", "CommandName or ALL"},
    {"HEALTH", AcpCommandType::Health, "Diagnostics", "HEALTH, HEALTH ON, HEALTH OFF, or HEALTH INTERVAL <milliseconds>", "Get/Set health telemetry configuration", "Implemented", 1, 2, "Subcommand and optional parameters", "ON, OFF, or INTERVAL <milliseconds>"}
};

const RegisterItem* CommandRegistry::findByName(const char* name) {
    for (size_t i = 0; i < TotalCommands; i++) {
        if (Items[i].name != nullptr && strcmp(Items[i].name, name) == 0) {
            return &Items[i];
        }
    }
    return nullptr;
}

const RegisterItem* CommandRegistry::findByType(AcpCommandType type) {
    for (size_t i = 0; i < TotalCommands; i++) {
        if (Items[i].type == type) {
            return &Items[i];
        }
    }
    return nullptr;
}

} // namespace amber
