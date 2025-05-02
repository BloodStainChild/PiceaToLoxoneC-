#pragma once
#ifndef LOXONE_API_H
#define LOXONE_API_H

#include <string>
#include <functional>
#include <future>
#include "PiceaAPI.h" // Enthält Definitionen von PiceaData und PiceaSettingData

class LoxoneAPI {
public:
    LoxoneAPI();
    ~LoxoneAPI();

    // Abonniert den PiceaAPI-Daten-Callback
    void SubscribeToPiceaAPI();

    // Wird aufgerufen, wenn neue Daten von PiceaAPI vorliegen
    void HandleFetchedData(const PiceaData& data, const PiceaSettingData& settings);

    // Sendet einen GET-Request an Loxone; virtueller Eingang und Wert werden übergeben
    bool SendDataToLoxone(const std::string& virtuellerEingang, const std::string& value);

    // Startet das Monitoring des virtuellen Ausgangs (blockierende Endlosschleife)
    void StartMonitoringLoxone();

    // Neu 
    void StartHttpServer();

    // Sendet die Daten an Loxone wenn sie unterschiedlich sind
    void CheckLoxoneValues();

private:
    // Prüft den Status eines virtuellen Ausgangs
    std::string CheckVirtualOutputStatus(const std::string& output);

    // Extrahiert den Status (Wert) aus der JSON-Antwort
    std::string ExtractStatusValue(const std::string& responseContent);

    // Extrahiert den Statuscode aus der JSON-Antwort und vergleicht mit "200"
    bool ExtractStatusCode(const std::string& responseContent);

    // Führt einen HTTP GET-Request aus und liefert die Antwort als String zurück
    bool HttpGet(const std::string& url, std::string& response);

    // Eine einfache Base64-Codierung (für die Basic-Auth)
    std::string Base64Encode(const std::string& in);

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    static int zeitInSekunden(const std::string& zeit);

    static std::string sekundenInZeitformat(int sekunden);
};

#endif // LOXONE_API_H
