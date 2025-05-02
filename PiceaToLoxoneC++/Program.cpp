#include <iostream>
#include <thread>
#include <chrono>
#include <exception>
#include <fstream>
#include <filesystem>
#include "Config.h"
#include "PiceaAPI.h"
#include "LoxoneAPI.h"
#include <ctime>
#include <iostream>

namespace fs = std::filesystem;

// Terminate-Handler, der bei unhandled exceptions aufgerufen wird
void TerminateHandler() {
    try {
        std::exception_ptr exptr = std::current_exception();
        if (exptr) {
            try {
                std::rethrow_exception(exptr);
            }
            catch (const std::exception& ex) {
                std::cerr << "Unhandled exception: " << ex.what() << std::endl;
                fs::create_directory("Log");
                std::ofstream logFile("Log/ERROR_Log.txt", std::ios_base::app);
                if (logFile.is_open()) {
                    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    char timeStr[26]; // Buffer für die Zeitzeichenkette
                    // Plattformunabhängige Zeitformatierung
#ifdef _WIN32
                    if (ctime_s(timeStr, sizeof(timeStr), &now) == 0)
#else
                    if (ctime_r(&now, timeStr) != nullptr)
#endif
                    {
                        // Entferne das '\n' am Ende, falls vorhanden
                        std::string timeString(timeStr);
                        if (!timeString.empty() && timeString.back() == '\n')
                            timeString.pop_back();
                        logFile << "[" << timeString << "] Unhandled Exception: " << ex.what() << std::endl;
                    }
                    logFile.close();
                }
            }
        }
        else {
            std::cerr << "Unhandled unknown exception." << std::endl;
        }
    }
    catch (...) {
        std::cerr << "Exception in TerminateHandler." << std::endl;
    }
    std::cerr << "Ein Fehler wurde erkannt. Der Server wird in 5 Sekunden heruntergefahren." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::abort();
}

int main() {
    std::set_terminate(TerminateHandler);

    std::cout << "PiceaToLoxoneC++ - Version 1.1" << std::endl << std::flush;
	std::cout << "Entwickelt von: Jens M" << std::endl;
	std::cout << "Copyright (c) 2023" << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	std::cout << "Starte Anwendung..." << std::endl;
	std::cout << "----------------------------------------" << std::endl;
    try {
        // Konfigurationsdatei laden
        if (!Config::LoadConfig()) {
            std::cout << "Config konnte nicht geladen werden. Anwendung wird beendet." << std::endl;
            return 1;
        }

        // LoxoneAPI instanziieren und an PiceaAPI binden
        LoxoneAPI loxoneAPI;
        loxoneAPI.SubscribeToPiceaAPI();

        // Starte PiceaAPI und LoxoneAPI in separaten Threads
        std::thread piceaThread([]() {
            PiceaAPI::StartLoop();
            });

		//// Starte den Loxone-Monitoring-Thread (OLD)
  //      std::thread loxoneThread([&loxoneAPI]() {
  //          loxoneAPI.StartMonitoringLoxone();
  //          });

        // Starte den HTTP-Server für Loxone Push
        std::thread httpServerThread([&loxoneAPI]() {
            loxoneAPI.StartHttpServer(); // HTTP-Server starten
            });

        // Warten, bis beide Threads enden (die Loops laufen normalerweise endlos)
        piceaThread.join();
        //loxoneThread.join(); // OLD
        httpServerThread.join();  // Warten auf HTTP-Server-Thread
    }
    catch (const std::exception& ex) {
        std::cerr << "Fehler beim Starten der Anwendung: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
