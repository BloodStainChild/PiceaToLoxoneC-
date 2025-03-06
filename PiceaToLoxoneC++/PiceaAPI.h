#pragma once
#ifndef PICEA_API_H
#define PICEA_API_H

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <chrono>
#include <stdexcept>



// -----------------------------
// Datenstrukturen (basierend auf PiceaData.cs)
// -----------------------------

// Einfaches Gerät
struct PairedDevice {
    int Id;
    std::string Name;
    std::chrono::system_clock::time_point paired_since;
};

// Wrapper für Werttypen (ähnlich ValueWrapper in C#)
template<typename T>
struct ValWrapper {
    std::optional<T> val;
};

// Wrapper, der einen ValWrapper enthält (ähnlich OkWrapper in C#)
template<typename T>
struct OkWrapper {
    ValWrapper<T> ok;
};

// PiceaData – alle Felder als OkWrapper<T>
struct PiceaData {
    OkWrapper<double> autarky;
    OkWrapper<double> battery_input_power;
    OkWrapper<double> battery_output_power;
    OkWrapper<double> battery_power;
    OkWrapper<double> battery_state_of_charge;
    OkWrapper<double> co2_avoidance;
    OkWrapper<double> electrolyser_efficiency_electrical;
    OkWrapper<double> electrolyser_efficiency_total;
    OkWrapper<double> electrolyser_heating_power;
    OkWrapper<double> electrolyser_output_power;
    OkWrapper<double> extbattery_input_power;
    OkWrapper<double> extbattery_output_power;
    OkWrapper<double> extbattery_power;
    OkWrapper<double> fuelcell_efficiency_electrical;
    OkWrapper<double> fuelcell_efficiency_total;
    OkWrapper<double> fuelcell_heating_power;
    OkWrapper<double> fuelcell_input_power;
    OkWrapper<double> grid_export_power;
    OkWrapper<double> grid_import_power;
    OkWrapper<double> grid_power;
    OkWrapper<double> heat_contribution_power;
    OkWrapper<double> hot_water_tempearture;
    OkWrapper<double> house_to_picea_air_humidity;
    OkWrapper<double> house_to_picea_air_temperature;
    OkWrapper<double> household_input_power;
    OkWrapper<double> household_input_power_from_battery_picea;
    OkWrapper<double> household_input_power_from_extbattery;
    OkWrapper<double> household_input_power_from_grid;
    OkWrapper<double> household_input_power_from_hydrogen;
    OkWrapper<double> household_input_power_from_picea;
    OkWrapper<double> hydrogen_input_power_electrical;
    OkWrapper<double> hydrogen_output_power_electrical;
    OkWrapper<double> hydrogen_power_electrical;
    OkWrapper<double> hydrogen_state_of_charge;
    OkWrapper<bool> is_efficiencymode;
    OkWrapper<bool> is_equal_charge_from_net;
    OkWrapper<bool> is_equal_charge_ongoing;
    OkWrapper<bool> is_grid_offline;
    OkWrapper<bool> is_heater_alarm;
    OkWrapper<bool> is_heater_error;
    OkWrapper<bool> is_hydrogen_used_except_reserve;
    OkWrapper<bool> is_in_ten_percent_grid_feedin_mode;
    OkWrapper<bool> is_launchphase;
    OkWrapper<bool> is_solar_error;
    OkWrapper<bool> is_ventilation_calibration_now;
    OkWrapper<bool> is_ventilation_differential_pressure_alarm;
    OkWrapper<bool> is_ventilation_filter_full_alarm;
    OkWrapper<bool> is_ventilation_filter_full_warning;
    OkWrapper<bool> is_water_error;
    OkWrapper<int> max_compressor_blockage_duration;
    OkWrapper<double> outdoor_to_picea_air_temperature;
    OkWrapper<double> picea_to_house_air_temperature;
    OkWrapper<double> solar_output_power;
    OkWrapper<double> solar_output_power_ac;
    OkWrapper<double> solar_output_power_dc_total;
    OkWrapper<double> solar_output_power_to_battery;
    OkWrapper<double> solar_output_power_to_extbattery;
    OkWrapper<double> solar_output_power_to_grid;
    OkWrapper<double> solar_output_power_to_household;
    OkWrapper<double> solar_output_power_to_hydrogen;
    OkWrapper<int> ventilation_stage_real;
};

// Enums (entsprechend den C#-Definitionen)
enum class PiceaGeneration {
    P1 = 0,
    P2 = 1,
    P3 = 2
};

enum class FilterExchangeState {
    init = 0,
    filterchange_currently_possible = 1,
    filterchange_preparing = 2,
    ready_for_filterchange = 3,
    filterchange_verifying = 4,
    filterchange_verification_failed = 5,
    filterchange_currently_NOT_possible = 6
};

enum class ExtBatterySetup {
    none = 0,
    Ext_basic = 1,
    Ext_hybrid = 2
};

// Wrapper für Werttypen (ValueWrapperValueType und ValueWrapperReferenceType)
// Wir nutzen hier std::optional als Ersatz
template<typename T>
struct ValueWrapperValueType {
    std::optional<T> value;
};


template <typename T>
class ValueWrapperReferenceType {
public:
    std::optional<T> value;  // Wert als optional

    ValueWrapperReferenceType() : value(std::nullopt) {}

    ValueWrapperReferenceType(const T& val, bool edit = false) : value(val) {}

    void setValue(const T& val) {
        value = val;
    }

    T getValue() const {
        if (value) {
            return *value;
        }
        throw std::runtime_error("Kein Wert gesetzt");
    }
};

// PiceaSettingData – Felder mit den oben definierten Wrappern
struct PiceaSettingData {
    ValueWrapperValueType<bool> party_mode_enabled;
    ValueWrapperValueType<bool> vacation_mode_enabled;
    ValueWrapperValueType<bool> has_grid_tax_feedin;
    ValueWrapperValueType<bool> is_grid_connected_system;
    ValueWrapperValueType<bool> has_no_hot_water_integrated;
    ValueWrapperValueType<bool> has_differential_pressure_gauge;
    ValueWrapperValueType<double> hydrogen_reserve;
    ValueWrapperReferenceType<ExtBatterySetup> ext_battery_setup;
    ValueWrapperReferenceType<FilterExchangeState> filter_exchange_state;
    ValueWrapperValueType<int> ventilation_stage_user;
    ValueWrapperValueType<double> ventilation_temperature_target;
    ValueWrapperValueType<bool> ventilation_night_enabled;
    ValueWrapperValueType<int> ventilation_night_stage;
    ValueWrapperReferenceType<std::string> ventilation_night_time_end;
    ValueWrapperReferenceType<std::string> ventilation_night_time_start;
    ValueWrapperValueType<int> compressor_blockage_duration;
    ValueWrapperValueType<bool> is_surpluspower_heatpump_enabled;
    ValueWrapperValueType<bool> is_surpluspower_grid_export_enabled;
    ValueWrapperValueType<bool> is_surpluspower_immersionheater_enabled;
    // Not editable
    ValueWrapperReferenceType<PiceaGeneration> picea_generation;
    ValueWrapperValueType<double> picea_count;
    ValueWrapperReferenceType<std::string> picea_serial;
    ValueWrapperValueType<bool> has_solar_dc;
    ValueWrapperValueType<bool> has_solar_ac;
    ValueWrapperValueType<bool> has_solar_ac_and_dc;
    ValueWrapperValueType<bool> has_sgready;
    ValueWrapperValueType<bool> has_no_heat_contribution;
    ValueWrapperValueType<bool> has_immersion_heater;
    ValueWrapperValueType<bool> has_ventilation_integrated;
};

// -----------------------------
// PiceaAPI Klasse
// -----------------------------

class PiceaAPI
{
public:
    // Callback, der nach erfolgreichem Datenabruf ausgelöst wird
    static std::function<void(PiceaData, PiceaSettingData)> OnDataFetched;

    // Statusvariablen
    static bool isConnected;
    static bool isFatalError;

    // Startet den API-Zyklus (Loop)
    static bool StartLoop();

    // Sendet aktualisierte Einstellungen an die API
    static void SendSettingsData(const PiceaSettingData& settings);

    static PiceaSettingData PSD;

private:
    static bool TryToConnect();
    static bool FetchData();
    // Holt Daten von der API und speichert die Antwort in responseBody
    static bool FetchFromApi(const std::string& apiUrl, std::string& responseBody);
    // Sendet per POST aktualisierte Einstellungen
    static bool UpdateConfigSettings(const std::string& jsonData);

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // PICEA_API_H

