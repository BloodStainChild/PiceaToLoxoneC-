#include <iostream>
#include <thread>
#include <chrono>
#include <exception>
#include <filesystem>
#include <future>
#include <string>
#include <ctime>
#include "Config.h"
#include "Logger.h"
#include "PiceaAPI.h"
#include "LoxoneAPI.h"

namespace fs = std::filesystem;

static std::string DetectPlatform()
{
#ifdef _WIN32
    return "windows";
#elif defined(__linux__)
    return "linux";
#elif defined(__APPLE__)
    return "macos";
#else
    return "unknown";
#endif
}

static std::string DetectArchitecture()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x64";
#elif defined(__i386) || defined(_M_IX86)
    return "x86";
#else
    return "unknown";
#endif
}

void TerminateHandler()
{
    try
    {
        std::exception_ptr exptr = std::current_exception();
        if (exptr)
        {
            try
            {
                std::rethrow_exception(exptr);
            }
            catch (const std::exception& ex)
            {
                Logger::Error("Program", "Unhandled exception: " + std::string(ex.what()));
            }
        }
        else
        {
            Logger::Error("Program", "Unhandled unknown exception.");
        }
    }
    catch (...)
    {
        Logger::Error("Program", "Exception inside terminate handler.");
    }

    Logger::Error("Program", "Fatal error detected. The application will abort in 5 seconds.");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::abort();
}

static std::string FormatLocalTime(const std::chrono::system_clock::time_point& tp)
{
    const std::time_t timeValue = std::chrono::system_clock::to_time_t(tp);
    std::tm localTm{};
#ifdef _WIN32
    localtime_s(&localTm, &timeValue);
#else
    localtime_r(&timeValue, &localTm);
#endif

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTm);
    return std::string(buffer);
}

static std::chrono::system_clock::time_point GetNextLocalMidnight()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTm{};
#ifdef _WIN32
    localtime_s(&localTm, &nowTime);
#else
    localtime_r(&nowTime, &localTm);
#endif

    localTm.tm_hour = 0;
    localTm.tm_min = 0;
    localTm.tm_sec = 0;
    localTm.tm_mday += 1;

    return std::chrono::system_clock::from_time_t(std::mktime(&localTm));
}

static void CleanupOldLogFiles()
{
    try
    {
        const fs::path logDir(Logger::GetLogDirectory());
        if (!fs::exists(logDir) || !fs::is_directory(logDir))
        {
            return;
        }

        const auto now = fs::file_time_type::clock::now();
        const auto maxAge = std::chrono::hours(24 * 365);

        for (const auto& entry : fs::directory_iterator(logDir))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            std::error_code ec;
            const auto lastWriteTime = fs::last_write_time(entry.path(), ec);
            if (ec)
            {
                Logger::Warn("Maintenance", "Could not read last write time for " + entry.path().string() + ": " + ec.message());
                continue;
            }

            if ((now - lastWriteTime) > maxAge)
            {
                const std::string filePath = entry.path().string();
                if (fs::remove(entry.path(), ec))
                {
                    Logger::Info("Maintenance", "Deleted old log file: " + filePath);
                }
                else if (ec)
                {
                    Logger::Warn("Maintenance", "Could not delete old log file " + filePath + ": " + ec.message());
                }
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::Error("Maintenance", "CleanupOldLogFiles failed: " + std::string(ex.what()));
    }
}

static bool WaitUntilRestartWindow(std::future<bool>& piceaFuture, const std::chrono::system_clock::time_point& restartAt)
{
    while (std::chrono::system_clock::now() < restartAt)
    {
        if (piceaFuture.wait_for(std::chrono::seconds(1)) == std::future_status::ready)
        {
            return false;
        }
    }

    return true;
}

int main()
{
    std::set_terminate(TerminateHandler);

    try
    {
        Logger::Info("Startup", "PiceaToLoxoneC++ Version 1.1 starting");
        Logger::Info("Startup", "Platform=" + DetectPlatform() + " Arch=" + DetectArchitecture());
        Logger::Info("Startup", "WorkingDir=" + fs::current_path().string());
        Logger::Info("Startup", "LogDir=" + Logger::GetLogDirectory());

        if (!Config::LoadConfig())
        {
            Logger::Error("Program", "Config could not be loaded. Application will exit.");
            return 1;
        }

        Logger::Info("Startup", "Configured poll interval=" + std::to_string(Config::PollIntervalSeconds) + "s");

        CleanupOldLogFiles();

        std::uint64_t serviceCycle = 0;
        while (true)
        {
            ++serviceCycle;
            Logger::Info("Program", "Starting internal service cycle #" + std::to_string(serviceCycle));

            PiceaAPI::ResetRuntimeState();

            LoxoneAPI loxoneAPI;
            loxoneAPI.SubscribeToPiceaAPI();

            if (!loxoneAPI.StartHttpServer())
            {
                Logger::Error("Program", "HTTP server could not be started. Retrying in 10 seconds.");
                std::this_thread::sleep_for(std::chrono::seconds(10));
                continue;
            }

            std::future<bool> piceaFuture = std::async(std::launch::async, []()
            {
                return PiceaAPI::StartLoop();
            });

            const auto nextMidnight = GetNextLocalMidnight();
            Logger::Info("Maintenance", "Next internal restart scheduled for " + FormatLocalTime(nextMidnight));

            const bool reachedRestartTime = WaitUntilRestartWindow(piceaFuture, nextMidnight);

            if (reachedRestartTime)
            {
                Logger::Info("Maintenance", "Midnight reached. Stopping internal services for restart.");
            }
            else
            {
                Logger::Warn("Program", "Picea polling loop ended before midnight. Internal services will be restarted now.");
            }

            CleanupOldLogFiles();

            PiceaAPI::RequestStop();
            loxoneAPI.StopHttpServer();

            try
            {
                piceaFuture.wait();
            }
            catch (const std::exception& ex)
            {
                Logger::Error("Program", "Polling loop shutdown wait failed: " + std::string(ex.what()));
            }

            Logger::Info("Program", "Internal service cycle #" + std::to_string(serviceCycle) + " stopped.");

            std::this_thread::sleep_for(std::chrono::seconds(2));
            Logger::Info("Program", "Restarting internal services.");
        }
    }
    catch (const std::exception& ex)
    {
        Logger::Error("Program", "Startup failed: " + std::string(ex.what()));
        return 1;
    }

    return 0;
}
