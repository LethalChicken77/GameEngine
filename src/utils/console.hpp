#pragma once
#include <string>
#include <queue>

class Console
{
    struct ConsoleMessage
    {
        std::string message;
        enum Type
        {
            NONE = 0,
            INFO = 1,
            DEBUG = 2,
            WARNING = 3,
            ERROR = 4
        } type;
    };

    public:
        static void logRaw(const std::string &message, bool terminalOnly = false);
        static void log(const std::string &message, const std::string &source = "", bool terminalOnly = false);
        static void debug(const std::string &message, const std::string &source = "", bool terminalOnly = false);
        static void warn(const std::string &message, const std::string &source = "", bool terminalOnly = false);
        static void error(const std::string &message, const std::string &source = "", bool terminalOnly = false);
        static void clear();
        static void drawImGui();
    private:
        static const size_t maxMessages = 100; // If exceeded, remove oldest
        static std::queue<ConsoleMessage> messages;
        static bool scrollToBottom;

        static ConsoleMessage constructMessage(const std::string& message, const std::string& source, ConsoleMessage::Type type);
        static void pushMessage(ConsoleMessage& message);

        static constexpr std::string consoleEndl = "\033[0m\n";
        static constexpr std::string ANSIreset = "\033[0m";
        static constexpr std::string ANSIred = "\033[31m";
        static constexpr std::string ANSIyellow = "\033[33m";
        static constexpr std::string ANSIgray = "\033[90m";
};