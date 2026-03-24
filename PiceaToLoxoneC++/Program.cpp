#include <iostream>
#include <thread>
#include <chrono>
#include <exception>
#include <filesystem>
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

        LoxoneAPI loxoneAPI;
        loxoneAPI.SubscribeToPiceaAPI();

        std::thread piceaThread([]()
        {
            PiceaAPI::StartLoop();
        });

        std::thread httpServerThread([&loxoneAPI]()
        {
            loxoneAPI.StartHttpServer();
        });

        piceaThread.join();
        httpServerThread.join();
    }
    catch (const std::exception& ex)
    {
        Logger::Error("Program", "Startup failed: " + std::string(ex.what()));
        return 1;
    }

    return 0;
}
