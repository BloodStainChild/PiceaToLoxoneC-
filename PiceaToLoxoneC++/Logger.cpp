#include "Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <cstdlib>
#include <cctype>

namespace
{
    std::string ReadEnvironmentVariable(const char* name)
    {
#ifdef _WIN32
        char* buffer = nullptr;
        size_t length = 0;
        if (_dupenv_s(&buffer, &length, name) == 0 && buffer != nullptr)
        {
            std::string value(buffer);
            free(buffer);
            return value;
        }
        return "";
#else
        const char* value = std::getenv(name);
        return (value != nullptr) ? std::string(value) : std::string();
#endif
    }
}

namespace fs = std::filesystem;

static std::mutex g_logMutex;

std::string Logger::GetTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);

    std::tm tmLocal{};
#ifdef _WIN32
    localtime_s(&tmLocal, &timeT);
#else
    localtime_r(&timeT, &tmLocal);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tmLocal, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::GetLogDirectory()
{
    const std::string envLogDir = ReadEnvironmentVariable("LOG_DIR");
    if (!envLogDir.empty())
    {
        return envLogDir;
    }
    return "Log";
}

bool Logger::IsDebugEnabled()
{
    std::string value = ReadEnvironmentVariable("LOG_DEBUG");
    if (value.empty())
    {
        return false;
    }

    for (char& c : value)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    return value == "1" || value == "true" || value == "yes" || value == "on";
}

void Logger::WriteLine(const std::string& fileName, const std::string& line)
{
    std::lock_guard<std::mutex> lock(g_logMutex);

    fs::create_directories(GetLogDirectory());

    std::ofstream logFile(GetLogDirectory() + "/" + fileName, std::ios::app);
    if (logFile.is_open())
    {
        logFile << line << std::endl;
    }
}

void Logger::Log(const std::string& level, const std::string& component, const std::string& message, bool toStdErr, const std::string& fileName)
{
    std::string line = "[" + GetTimestamp() + "] [" + level + "]";
    if (!component.empty())
    {
        line += " [" + component + "]";
    }
    line += " " + message;

    if (toStdErr)
    {
        std::cerr << line << std::endl;
    }
    else
    {
        std::cout << line << std::endl;
    }

    WriteLine(fileName, line);
}

void Logger::Info(const std::string& message)
{
    Log("INFO", "", message, false);
}

void Logger::Info(const std::string& component, const std::string& message)
{
    Log("INFO", component, message, false);
}

void Logger::Warn(const std::string& message)
{
    Log("WARN", "", message, false);
}

void Logger::Warn(const std::string& component, const std::string& message)
{
    Log("WARN", component, message, false);
}

void Logger::Error(const std::string& message)
{
    Log("ERROR", "", message, true);
}

void Logger::Error(const std::string& component, const std::string& message)
{
    Log("ERROR", component, message, true);
}

void Logger::Debug(const std::string& message)
{
    if (IsDebugEnabled())
    {
        Log("DEBUG", "", message, false);
    }
}

void Logger::Debug(const std::string& component, const std::string& message)
{
    if (IsDebugEnabled())
    {
        Log("DEBUG", component, message, false);
    }
}

void Logger::Change(const std::string& settingName, const std::string& oldValue, const std::string& newValue)
{
    Change("generic", settingName, oldValue, newValue, "changed");
}

void Logger::Change(const std::string& channel, const std::string& settingName, const std::string& oldValue, const std::string& newValue, const std::string& result)
{
    std::string line = "[" + GetTimestamp() + "] [CHANGE] [" + channel + "] setting=" + settingName
        + " old=" + oldValue
        + " new=" + newValue
        + " result=" + result;

    std::cout << line << std::endl;
    WriteLine("loxone_changes.log", line);
    WriteLine("App.log", line);
}
