#pragma once
#include <string>

class Logger
{
public:
    static void Info(const std::string& message);
    static void Info(const std::string& component, const std::string& message);

    static void Warn(const std::string& message);
    static void Warn(const std::string& component, const std::string& message);

    static void Error(const std::string& message);
    static void Error(const std::string& component, const std::string& message);

    static void Debug(const std::string& message);
    static void Debug(const std::string& component, const std::string& message);

    static void Change(const std::string& settingName, const std::string& oldValue, const std::string& newValue);
    static void Change(const std::string& channel, const std::string& settingName, const std::string& oldValue, const std::string& newValue, const std::string& result = "changed");

    static std::string GetLogDirectory();

private:
    static std::string GetTimestamp();
    static bool IsDebugEnabled();
    static void Log(const std::string& level, const std::string& component, const std::string& message, bool toStdErr = false, const std::string& fileName = "App.log");
    static void WriteLine(const std::string& fileName, const std::string& line);
};
