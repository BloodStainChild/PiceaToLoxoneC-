#include "LoxoneAPI.h"
#include "Config.h"
#include "Logger.h"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <cstdio>
#include <cctype>
#include <fstream> // Für das Schreiben in Dateien
#include <cstring>

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static std::string RedactUrl(const std::string& url)
{
    std::string redacted = url;

    std::size_t schemePos = redacted.find("://");
    if (schemePos != std::string::npos)
    {
        std::size_t userInfoStart = schemePos + 3;
        std::size_t atPos = redacted.find('@', userInfoStart);
        if (atPos != std::string::npos)
        {
            redacted.erase(userInfoStart, atPos - userInfoStart + 1);
        }
    }

    return redacted;
}

int LoxoneAPI::zeitInSekunden(const std::string& zeit) {
    int stunden, minuten, sekunden;
    char delimiter1, delimiter2;
    std::istringstream iss(zeit);

    if (iss >> stunden >> delimiter1 >> minuten >> delimiter2 >> sekunden
        && delimiter1 == ':' && delimiter2 == ':') {
        return stunden * 3600 + minuten * 60 + sekunden;
    }
    return 0; // Fehlercode, falls das Format ungültig ist
}

std::string LoxoneAPI::sekundenInZeitformat(int sekunden) {
    if (sekunden < 0) sekunden = 0; // Negative Werte verhindern

    sekunden %= 86400; // Begrenzung auf 24 Stunden (86400 Sekunden)

    int stunden = sekunden / 3600;
    sekunden %= 3600;
    int minuten = sekunden / 60;
    sekunden %= 60;

    char zeit[9];
    snprintf(zeit, sizeof(zeit), "%02d:%02d:%02d", stunden, minuten, sekunden);

    return std::string(zeit);
}

LoxoneAPI::LoxoneAPI() {
    // Initialisiere libcurl global
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

LoxoneAPI::~LoxoneAPI() {
    StopHttpServer();
    curl_global_cleanup();
}

void LoxoneAPI::SubscribeToPiceaAPI() {
    Logger::Info("LoxoneAPI", "Subscribed to Picea data callback.");
    PiceaAPI::OnDataFetched = [this](const PiceaData& data, const PiceaSettingData& settings) {
        this->HandleFetchedData(data, settings);
    };
}

void LoxoneAPI::HandleFetchedData(const PiceaData& data, const PiceaSettingData& settings) {

    // In der C#-Version werden mehrere asynchrone Tasks gestartet.
    // Hier nutzen wir std::async, um parallele Aufrufe von SendDataToLoxone zu starten.
    std::vector<std::future<bool>> tasks;

    // Beispielhafte Aufrufe – hier wird angenommen, dass in den Wrappern die Werte als std::optional<double> vorliegen.
    if (data.autarky.ok.val.has_value() && !Config::autarky.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::autarky, std::to_string(data.autarky.ok.val.value())));
    if (data.battery_input_power.ok.val.has_value() && !Config::battery_input_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::battery_input_power, std::to_string(data.battery_input_power.ok.val.value())));
    if (data.battery_output_power.ok.val.has_value() && !Config::battery_output_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::battery_output_power, std::to_string(data.battery_output_power.ok.val.value())));
    if (data.battery_power.ok.val.has_value() && !Config::battery_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::battery_power, std::to_string(data.battery_power.ok.val.value())));
    if (data.battery_state_of_charge.ok.val.has_value() && !Config::battery_state_of_charge.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::battery_state_of_charge, std::to_string(data.battery_state_of_charge.ok.val.value())));
    if (data.co2_avoidance.ok.val.has_value() && !Config::co2_avoidance.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::co2_avoidance, std::to_string(data.co2_avoidance.ok.val.value())));
    if (data.electrolyser_efficiency_electrical.ok.val.has_value() && !Config::electrolyser_efficiency_electrical.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::electrolyser_efficiency_electrical, std::to_string(data.electrolyser_efficiency_electrical.ok.val.value())));
    if (data.electrolyser_efficiency_total.ok.val.has_value() && !Config::electrolyser_efficiency_total.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::electrolyser_efficiency_total, std::to_string(data.electrolyser_efficiency_total.ok.val.value())));
    if (data.electrolyser_heating_power.ok.val.has_value() && !Config::electrolyser_heating_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::electrolyser_heating_power, std::to_string(data.electrolyser_heating_power.ok.val.value())));
    if (data.electrolyser_output_power.ok.val.has_value() && !Config::electrolyser_output_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::electrolyser_output_power, std::to_string(data.electrolyser_output_power.ok.val.value())));
    if (data.extbattery_input_power.ok.val.has_value() && !Config::extbattery_input_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::extbattery_input_power, std::to_string(data.extbattery_input_power.ok.val.value())));
    if (data.extbattery_output_power.ok.val.has_value() && !Config::extbattery_output_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::extbattery_output_power, std::to_string(data.extbattery_output_power.ok.val.value())));
    if (data.extbattery_power.ok.val.has_value() && !Config::extbattery_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::extbattery_power, std::to_string(data.extbattery_power.ok.val.value())));
    if (data.fuelcell_efficiency_electrical.ok.val.has_value() && !Config::fuelcell_efficiency_electrical.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::fuelcell_efficiency_electrical, std::to_string(data.fuelcell_efficiency_electrical.ok.val.value())));
    if (data.fuelcell_efficiency_total.ok.val.has_value() && !Config::fuelcell_efficiency_total.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::fuelcell_efficiency_total, std::to_string(data.fuelcell_efficiency_total.ok.val.value())));
    if (data.fuelcell_heating_power.ok.val.has_value() && !Config::fuelcell_heating_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::fuelcell_heating_power, std::to_string(data.fuelcell_heating_power.ok.val.value())));
    if (data.fuelcell_input_power.ok.val.has_value() && !Config::fuelcell_input_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::fuelcell_input_power, std::to_string(data.fuelcell_input_power.ok.val.value())));
    if (data.grid_export_power.ok.val.has_value() && !Config::grid_export_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::grid_export_power, std::to_string(data.grid_export_power.ok.val.value())));
    if (data.grid_import_power.ok.val.has_value() && !Config::grid_import_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::grid_import_power, std::to_string(data.grid_import_power.ok.val.value())));
    if (data.grid_power.ok.val.has_value() && !Config::grid_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::grid_power, std::to_string(data.grid_power.ok.val.value())));
    if (data.heat_contribution_power.ok.val.has_value() && !Config::heat_contribution_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::heat_contribution_power, std::to_string(data.heat_contribution_power.ok.val.value())));
    if (data.hot_water_tempearture.ok.val.has_value() && !Config::hot_water_tempearture.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::hot_water_tempearture, std::to_string(data.hot_water_tempearture.ok.val.value())));
    if (data.house_to_picea_air_humidity.ok.val.has_value() && !Config::house_to_picea_air_humidity.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::house_to_picea_air_humidity, std::to_string(data.house_to_picea_air_humidity.ok.val.value())));
    if (data.house_to_picea_air_temperature.ok.val.has_value() && !Config::house_to_picea_air_temperature.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::house_to_picea_air_temperature, std::to_string(data.house_to_picea_air_temperature.ok.val.value())));
    if (data.household_input_power.ok.val.has_value() && !Config::household_input_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::household_input_power, std::to_string(data.household_input_power.ok.val.value())));
    if (data.household_input_power_from_battery_picea.ok.val.has_value() && !Config::household_input_power_from_battery_picea.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::household_input_power_from_battery_picea, std::to_string(data.household_input_power_from_battery_picea.ok.val.value())));
    if (data.household_input_power_from_extbattery.ok.val.has_value() && !Config::household_input_power_from_extbattery.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::household_input_power_from_extbattery, std::to_string(data.household_input_power_from_extbattery.ok.val.value())));
    if (data.household_input_power_from_grid.ok.val.has_value() && !Config::household_input_power_from_grid.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::household_input_power_from_grid, std::to_string(data.household_input_power_from_grid.ok.val.value())));
    if (data.household_input_power_from_hydrogen.ok.val.has_value() && !Config::household_input_power_from_hydrogen.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::household_input_power_from_hydrogen, std::to_string(data.household_input_power_from_hydrogen.ok.val.value())));
    if (data.household_input_power_from_picea.ok.val.has_value() && !Config::household_input_power_from_picea.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::household_input_power_from_picea, std::to_string(data.household_input_power_from_picea.ok.val.value())));
    if (data.hydrogen_input_power_electrical.ok.val.has_value() && !Config::hydrogen_input_power_electrical.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::hydrogen_input_power_electrical, std::to_string(data.hydrogen_input_power_electrical.ok.val.value())));
    if (data.hydrogen_output_power_electrical.ok.val.has_value() && !Config::hydrogen_output_power_electrical.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::hydrogen_output_power_electrical, std::to_string(data.hydrogen_output_power_electrical.ok.val.value())));
    if (data.hydrogen_power_electrical.ok.val.has_value() && !Config::hydrogen_power_electrical.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::hydrogen_power_electrical, std::to_string(data.hydrogen_power_electrical.ok.val.value())));
    if (data.hydrogen_state_of_charge.ok.val.has_value() && !Config::hydrogen_state_of_charge.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::hydrogen_state_of_charge, std::to_string(data.hydrogen_state_of_charge.ok.val.value())));
    if (data.is_efficiencymode.ok.val.has_value() && !Config::is_efficiencymode.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_efficiencymode, std::to_string(data.is_efficiencymode.ok.val.value())));
    if (data.is_equal_charge_from_net.ok.val.has_value() && !Config::is_equal_charge_from_net.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_equal_charge_from_net, std::to_string(data.is_equal_charge_from_net.ok.val.value())));
    if (data.is_equal_charge_ongoing.ok.val.has_value() && !Config::is_equal_charge_ongoing.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_equal_charge_ongoing, std::to_string(data.is_equal_charge_ongoing.ok.val.value())));
    if (data.is_grid_offline.ok.val.has_value() && !Config::is_grid_offline.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_grid_offline, std::to_string(data.is_grid_offline.ok.val.value())));
    if (data.is_heater_alarm.ok.val.has_value() && !Config::is_heater_alarm.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_heater_alarm, std::to_string(data.is_heater_alarm.ok.val.value())));
    if (data.is_heater_error.ok.val.has_value() && !Config::is_heater_error.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_heater_error, std::to_string(data.is_heater_error.ok.val.value())));
    if (data.is_hydrogen_used_except_reserve.ok.val.has_value() && !Config::is_hydrogen_used_except_reserve.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_hydrogen_used_except_reserve, std::to_string(data.is_hydrogen_used_except_reserve.ok.val.value())));
    if (data.is_in_ten_percent_grid_feedin_mode.ok.val.has_value() && !Config::is_in_ten_percent_grid_feedin_mode.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_in_ten_percent_grid_feedin_mode, std::to_string(data.is_in_ten_percent_grid_feedin_mode.ok.val.value())));
    if (data.is_launchphase.ok.val.has_value() && !Config::is_launchphase.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_launchphase, std::to_string(data.is_launchphase.ok.val.value())));
    if (data.is_solar_error.ok.val.has_value() && !Config::is_solar_error.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_solar_error, std::to_string(data.is_solar_error.ok.val.value())));
    if (data.is_ventilation_calibration_now.ok.val.has_value() && !Config::is_ventilation_calibration_now.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_ventilation_calibration_now, std::to_string(data.is_ventilation_calibration_now.ok.val.value())));
    if (data.is_ventilation_differential_pressure_alarm.ok.val.has_value() && !Config::is_ventilation_differential_pressure_alarm.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_ventilation_differential_pressure_alarm, std::to_string(data.is_ventilation_differential_pressure_alarm.ok.val.value())));
    if (data.is_ventilation_filter_full_alarm.ok.val.has_value() && !Config::is_ventilation_filter_full_alarm.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_ventilation_filter_full_alarm, std::to_string(data.is_ventilation_filter_full_alarm.ok.val.value())));
    if (data.is_ventilation_filter_full_warning.ok.val.has_value() && !Config::is_ventilation_filter_full_warning.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_ventilation_filter_full_warning, std::to_string(data.is_ventilation_filter_full_warning.ok.val.value())));
    if (data.is_water_error.ok.val.has_value() && !Config::is_water_error.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_water_error, std::to_string(data.is_water_error.ok.val.value())));
    if (data.max_compressor_blockage_duration.ok.val.has_value() && !Config::max_compressor_blockage_duration.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::max_compressor_blockage_duration, std::to_string(data.max_compressor_blockage_duration.ok.val.value())));
    if (data.outdoor_to_picea_air_temperature.ok.val.has_value() && !Config::outdoor_to_picea_air_temperature.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::outdoor_to_picea_air_temperature, std::to_string(data.outdoor_to_picea_air_temperature.ok.val.value())));
    if (data.picea_to_house_air_temperature.ok.val.has_value() && !Config::picea_to_house_air_temperature.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::picea_to_house_air_temperature, std::to_string(data.picea_to_house_air_temperature.ok.val.value())));
    if (data.solar_output_power.ok.val.has_value() && !Config::solar_output_power.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power, std::to_string(data.solar_output_power.ok.val.value())));
    if (data.solar_output_power_ac.ok.val.has_value() && !Config::solar_output_power_ac.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_ac, std::to_string(data.solar_output_power_ac.ok.val.value())));
    if (data.solar_output_power_dc_total.ok.val.has_value() && !Config::solar_output_power_dc_total.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_dc_total, std::to_string(data.solar_output_power_dc_total.ok.val.value())));
    if (data.solar_output_power_to_battery.ok.val.has_value() && !Config::solar_output_power_to_battery.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_to_battery, std::to_string(data.solar_output_power_to_battery.ok.val.value())));
    if (data.solar_output_power_to_extbattery.ok.val.has_value() && !Config::solar_output_power_to_extbattery.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_to_extbattery, std::to_string(data.solar_output_power_to_extbattery.ok.val.value())));
    if (data.solar_output_power_to_grid.ok.val.has_value() && !Config::solar_output_power_to_grid.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_to_grid, std::to_string(data.solar_output_power_to_grid.ok.val.value())));
    if (data.solar_output_power_to_household.ok.val.has_value() && !Config::solar_output_power_to_household.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_to_household, std::to_string(data.solar_output_power_to_household.ok.val.value())));
    if (data.solar_output_power_to_hydrogen.ok.val.has_value() && !Config::solar_output_power_to_hydrogen.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::solar_output_power_to_hydrogen, std::to_string(data.solar_output_power_to_hydrogen.ok.val.value())));
    if (data.ventilation_stage_real.ok.val.has_value() && !Config::ventilation_stage_real.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_stage_real, std::to_string(data.ventilation_stage_real.ok.val.value())));

    // Settings senden zu loxone
    if (settings.party_mode_enabled.value.has_value() && !Config::party_mode_enabled_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::party_mode_enabled_IN, std::to_string(settings.party_mode_enabled.value.value())));
    if (settings.vacation_mode_enabled.value.has_value() && !Config::vacation_mode_enabled_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::vacation_mode_enabled_IN, std::to_string(settings.vacation_mode_enabled.value.value())));
    if (settings.has_grid_tax_feedin.value.has_value() && !Config::has_grid_tax_feedin_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_grid_tax_feedin_IN, std::to_string(settings.has_grid_tax_feedin.value.value())));
    if (settings.is_grid_connected_system.value.has_value() && !Config::is_grid_connected_system_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_grid_connected_system_IN, std::to_string(settings.is_grid_connected_system.value.value())));
    if (settings.is_grid_connected_system.value.has_value() && !Config::is_grid_connected_system_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_grid_connected_system_IN, std::to_string(settings.is_grid_connected_system.value.value())));
    if (settings.has_no_hot_water_integrated.value.has_value() && !Config::has_no_hot_water_integrated_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_no_hot_water_integrated_IN, std::to_string(settings.has_no_hot_water_integrated.value.value())));
    if (settings.has_differential_pressure_gauge.value.has_value() && !Config::has_differential_pressure_gauge_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_differential_pressure_gauge_IN, std::to_string(settings.has_differential_pressure_gauge.value.value())));
    if (settings.hydrogen_reserve.value.has_value() && !Config::hydrogen_reserve_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::hydrogen_reserve_IN, std::to_string(settings.hydrogen_reserve.value.value())));

    if (settings.ext_battery_setup.value.has_value() && !Config::ext_battery_setup_IN.empty())
    {
        ExtBatterySetup val = settings.ext_battery_setup.getValue();
        int statusCode = static_cast<int>(val);
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ext_battery_setup_IN, std::to_string(statusCode)));
    }
    if (settings.filter_exchange_state.value.has_value() && !Config::filter_exchange_state_IN.empty())
    {
        FilterExchangeState val = settings.filter_exchange_state.getValue();
        int statusCode = static_cast<int>(val);
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::filter_exchange_state_IN, std::to_string(statusCode)));
    }
    if (settings.picea_generation.value.has_value() && !Config::picea_generation_IN.empty())
    {
        PiceaGeneration val = settings.picea_generation.getValue();
        int statusCode = static_cast<int>(val);

        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::picea_generation_IN, std::to_string(statusCode)));
    }

    if (settings.ventilation_stage_user.value.has_value() && !Config::ventilation_stage_user_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_stage_user_IN, std::to_string(settings.ventilation_stage_user.value.value())));
    if (settings.ventilation_temperature_target.value.has_value() && !Config::ventilation_temperature_target_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_temperature_target_IN, std::to_string(settings.ventilation_temperature_target.value.value())));
    if (settings.ventilation_night_enabled.value.has_value() && !Config::ventilation_night_enabled_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_night_enabled_IN, std::to_string(settings.ventilation_night_enabled.value.value())));
    if (settings.ventilation_night_stage.value.has_value() && !Config::ventilation_night_stage_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_night_stage_IN, std::to_string(settings.ventilation_night_stage.value.value())));
    if (settings.ventilation_night_time_start.value.has_value() && !Config::ventilation_night_time_start_IN.empty())
    {
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_night_time_start_IN, std::to_string(zeitInSekunden(settings.ventilation_night_time_start.value.value()))));
    }
    if (settings.ventilation_night_time_end.value.has_value() && !Config::ventilation_night_time_end_IN.empty())
    {
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::ventilation_night_time_end_IN, std::to_string(zeitInSekunden(settings.ventilation_night_time_end.value.value()))));
    }
    if (settings.compressor_blockage_duration.value.has_value() && !Config::compressor_blockage_duration_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::compressor_blockage_duration_IN, std::to_string(settings.compressor_blockage_duration.value.value())));
    if (settings.is_surpluspower_heatpump_enabled.value.has_value() && !Config::is_surpluspower_heatpump_enabled_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_surpluspower_heatpump_enabled_IN, std::to_string(settings.is_surpluspower_heatpump_enabled.value.value())));
    if (settings.is_surpluspower_grid_export_enabled.value.has_value() && !Config::is_surpluspower_grid_export_enabled_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_surpluspower_grid_export_enabled_IN, std::to_string(settings.is_surpluspower_grid_export_enabled.value.value())));
    if (settings.is_surpluspower_immersionheater_enabled.value.has_value() && !Config::is_surpluspower_immersionheater_enabled_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::is_surpluspower_immersionheater_enabled_IN, std::to_string(settings.is_surpluspower_immersionheater_enabled.value.value())));    
    if (settings.picea_count.value.has_value() && !Config::picea_count_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::picea_count_IN, std::to_string(settings.picea_count.value.value())));
    if (settings.picea_serial.value.has_value() && !Config::picea_serial_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::picea_serial_IN, settings.picea_serial.value.value()));
    if (settings.has_solar_dc.value.has_value() && !Config::has_solar_dc_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_solar_dc_IN, std::to_string(settings.has_solar_dc.value.value())));
    if (settings.has_solar_ac.value.has_value() && !Config::has_solar_ac_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_solar_ac_IN, std::to_string(settings.has_solar_ac.value.value())));
    if (settings.has_solar_ac_and_dc.value.has_value() && !Config::has_solar_ac_and_dc_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_solar_ac_and_dc_IN, std::to_string(settings.has_solar_ac_and_dc.value.value())));
    if (settings.has_sgready.value.has_value() && !Config::has_sgready_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_sgready_IN, std::to_string(settings.has_sgready.value.value())));
    if (settings.has_no_heat_contribution.value.has_value() && !Config::has_no_heat_contribution_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_no_heat_contribution_IN, std::to_string(settings.has_no_heat_contribution.value.value())));
    if (settings.has_immersion_heater.value.has_value() && !Config::has_immersion_heater_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_immersion_heater_IN, std::to_string(settings.has_immersion_heater.value.value())));
    if (settings.has_ventilation_integrated.value.has_value() && !Config::has_ventilation_integrated_IN.empty())
        tasks.push_back(std::async(std::launch::async, &LoxoneAPI::SendDataToLoxone, this, Config::has_ventilation_integrated_IN, std::to_string(settings.has_ventilation_integrated.value.value())));
   
    // Warte auf alle Aufgaben
    for (auto& t : tasks) {
        try {
            t.get();
        }
        catch (const std::exception& ex) {
            Logger::Error("LoxoneAPI", "One async send request failed: " + std::string(ex.what()));
        }
    }
}

bool LoxoneAPI::SendDataToLoxone(const std::string& virtuellerEingang, const std::string& value) {
    try {
        std::string url = "https://" + Config::LoxoneIP + "/jdev/sps/io/" + virtuellerEingang + "/" + value;
        std::string response;
        if (!HttpGet(url, response)) {
            Logger::Error("LoxoneAPI", "HTTP request to Loxone failed for input '" + virtuellerEingang + "'.");
            return false;
        }
        if (!ExtractStatusCode(response)) {
            Logger::Error("LoxoneAPI", "Loxone rejected value update for input '" + virtuellerEingang + "'. Response: " + response);
            return false;
        }

        Logger::Debug("LoxoneAPI", "Value sent successfully to '" + virtuellerEingang + "' with value '" + value + "'.");
        return true;
    }
    catch (const std::exception& ex) {
        Logger::Error("LoxoneAPI", "Exception while sending data to Loxone for input '" + virtuellerEingang + "': " + std::string(ex.what()));
        return false;
    }
}

void LoxoneAPI::CheckLoxoneValues() {
    // Hier wird der Status der virtuellen Ausgänge überprüft
    bool bSendFlag = false;
    PiceaSettingData newpsd = {};

    // Beispiel für das Logging
    auto logChange = [](const std::string& settingName, const std::string& oldValue, const std::string& newValue)
    {
        Logger::Change("Loxone->Picea", settingName, oldValue, newValue, "pending_send");
    };

    if (!Config::party_mode_enabled_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::party_mode_enabled_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.party_mode_enabled.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.party_mode_enabled.value = result;
                // Logge die Änderung
                logChange("party_mode_enabled", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::vacation_mode_enabled_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::vacation_mode_enabled_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.vacation_mode_enabled.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.vacation_mode_enabled.value = result;
				// Logge die Änderung
				logChange("vacation_mode_enabled", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::has_grid_tax_feedin_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::has_grid_tax_feedin_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.has_grid_tax_feedin.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.has_grid_tax_feedin.value = result;
				// Logge die Änderung
                logChange("has_grid_tax_feedin", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::is_grid_connected_system_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::is_grid_connected_system_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.is_grid_connected_system.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.is_grid_connected_system.value = result;
				// Logge die Änderung
				logChange("is_grid_connected_system", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::has_no_hot_water_integrated_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::has_no_hot_water_integrated_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.has_no_hot_water_integrated.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.has_no_hot_water_integrated.value = result;
				// Logge die Änderung
				logChange("has_no_hot_water_integrated", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::has_differential_pressure_gauge_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::has_differential_pressure_gauge_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.has_differential_pressure_gauge.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.has_differential_pressure_gauge.value = result;
				// Logge die Änderung
				logChange("has_differential_pressure_gauge", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::hydrogen_reserve_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::hydrogen_reserve_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.hydrogen_reserve.value;

            if (currentValue.has_value() && currentValue.value() != atof(value.c_str())) {
                bSendFlag = true;
                newpsd.hydrogen_reserve.value = atof(value.c_str());
				// Logge die Änderung
				logChange("hydrogen_reserve", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", value);
            }
        }
    }
    if (!Config::ext_battery_setup_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ext_battery_setup_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.ext_battery_setup.value;
            int iVal = atoi(value.c_str());

            if (currentValue.has_value() && currentValue.value() != static_cast<ExtBatterySetup>(iVal)) {
                bSendFlag = true;
                newpsd.ext_battery_setup.value = static_cast<ExtBatterySetup>(iVal);
				// Logge die Änderung
				logChange("ext_battery_setup", currentValue.has_value() ? std::to_string(static_cast<int>(currentValue.value())) : "null", value);
            }
        }
    }
    if (!Config::filter_exchange_state_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::filter_exchange_state_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.filter_exchange_state.value;
            int iVal = atoi(value.c_str());
			if (iVal == 2 || iVal == 4) { // Nur gültige Werte akzeptieren (2, 4)
                if (currentValue.has_value() && currentValue.value() != static_cast<FilterExchangeState>(iVal)) {
                    bSendFlag = true;
                    newpsd.filter_exchange_state.value = static_cast<FilterExchangeState>(iVal);
                    // Logge die Änderung
                    logChange("filter_exchange_state", currentValue.has_value() ? std::to_string(static_cast<int>(currentValue.value())) : "null", value);
                }
            }
        }
    }
    if (!Config::ventilation_stage_user_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ventilation_stage_user_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.ventilation_stage_user.value;

            if (currentValue.has_value() && currentValue.value() != atoi(value.c_str())) {
                bSendFlag = true;
                newpsd.ventilation_stage_user.value = atoi(value.c_str());
				// Logge die Änderung
				logChange("ventilation_stage_user", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", value);
            }
        }
    }
    if (!Config::ventilation_temperature_target_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ventilation_temperature_target_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.ventilation_temperature_target.value;

            if (currentValue.has_value() && currentValue.value() != atof(value.c_str())) {
                bSendFlag = true;
                newpsd.ventilation_temperature_target.value = atof(value.c_str());
				// Logge die Änderung
				logChange("ventilation_temperature_target", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", value);
            }
        }
    }
    if (!Config::ventilation_night_enabled_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ventilation_night_enabled_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.ventilation_night_enabled.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.ventilation_night_enabled.value = result;
				// Logge die Änderung
				logChange("ventilation_night_enabled", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::ventilation_night_stage_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ventilation_night_stage_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.ventilation_night_stage.value;

            if (currentValue.has_value() && currentValue.value() != atoi(value.c_str())) {
                bSendFlag = true;
                newpsd.ventilation_night_stage.value = atoi(value.c_str());
				// Logge die Änderung
				logChange("ventilation_night_stage", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", value);
            }
        }
    }
    if (!Config::ventilation_night_time_start_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ventilation_night_time_start_OUT);
        if (!value.empty())
        {
            std::string newtime = sekundenInZeitformat(atoi(value.c_str()));
            auto& currentValue = PiceaAPI::PSD.ventilation_night_time_start.value;

            if (currentValue.has_value() && currentValue.value() != newtime)
            {
                bSendFlag = true;
                newpsd.ventilation_night_time_start.value = newtime;
				// Logge die Änderung
				logChange("ventilation_night_time_start", currentValue.has_value() ? currentValue.value() : "null", newtime);
            }
        }
    }
    if (!Config::ventilation_night_time_end_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::ventilation_night_time_end_OUT);
        if (!value.empty())
        {
            std::string newtime = sekundenInZeitformat(atoi(value.c_str()));
            auto& currentValue = PiceaAPI::PSD.ventilation_night_time_end.value;

            if (currentValue.has_value() && currentValue.value() != newtime)
            {
                bSendFlag = true;
                newpsd.ventilation_night_time_end.value = newtime;
				// Logge die Änderung
				logChange("ventilation_night_time_end", currentValue.has_value() ? currentValue.value() : "null", newtime);
            }
        }
    }
    if (!Config::compressor_blockage_duration_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::compressor_blockage_duration_OUT);
        if (!value.empty())
        {
            auto& currentValue = PiceaAPI::PSD.compressor_blockage_duration.value;

            if (currentValue.has_value() && currentValue.value() != atoi(value.c_str())) {
                bSendFlag = true;
                newpsd.compressor_blockage_duration.value = atoi(value.c_str());
				// Logge die Änderung
				logChange("compressor_blockage_duration", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", value);
            }
        }
    }
    if (!Config::is_surpluspower_heatpump_enabled_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::is_surpluspower_heatpump_enabled_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.is_surpluspower_heatpump_enabled.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.is_surpluspower_heatpump_enabled.value = result;
				// Logge die Änderung
				logChange("is_surpluspower_heatpump_enabled", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::is_surpluspower_grid_export_enabled_OUT.empty()) {
        std::string value = CheckVirtualOutputStatus(Config::is_surpluspower_grid_export_enabled_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.is_surpluspower_grid_export_enabled.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.is_surpluspower_grid_export_enabled.value = result;
				// Logge die Änderung
				logChange("is_surpluspower_grid_export_enabled", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }
    if (!Config::is_surpluspower_immersionheater_enabled_OUT.empty())
    {
        std::string value = CheckVirtualOutputStatus(Config::is_surpluspower_immersionheater_enabled_OUT);
        if (!value.empty())
        {
            bool result = (value == "true" || value == "1");
            auto& currentValue = PiceaAPI::PSD.is_surpluspower_immersionheater_enabled.value;

            if (currentValue.has_value() && currentValue.value() != result) {
                bSendFlag = true;
                newpsd.is_surpluspower_immersionheater_enabled.value = result;
				// Logge die Änderung
				logChange("is_surpluspower_immersionheater_enabled", currentValue.has_value() ? std::to_string(currentValue.value()) : "null", std::to_string(result));
            }
        }
    }

    // Sende nur änderungen :)
    if (bSendFlag)
    {
        Logger::Info("LoxoneAPI", "Detected changed virtual outputs. Sending updated settings to Picea.");
        PiceaAPI::SendSettingsData(newpsd);
    }
}

std::string LoxoneAPI::CheckVirtualOutputStatus(const std::string& output) {
    try {
        std::string url = "https://" + Config::LoxoneUser + ":" + Config::LoxonePW + "@" + Config::LoxoneIP + "/jdev/sps/io/" + output;
        std::string response;
        if (!HttpGet(url, response)) {
            Logger::Error("LoxoneAPI", "Failed to query virtual output '" + output + "'.");
            return "";
        }
        std::string value = ExtractStatusValue(response);
        if (value.empty()) {
            Logger::Warn("LoxoneAPI", "No status value returned for virtual output '" + output + "'.");
            return "";
        }
        return value;
    }
    catch (const std::exception& ex) {
        Logger::Error("LoxoneAPI", "Exception while querying virtual output '" + output + "': " + std::string(ex.what()));
        return "";
    }
}

std::string LoxoneAPI::ExtractStatusValue(const std::string& responseContent) {
    try {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errs;
        std::istringstream iss(responseContent);
        if (!Json::parseFromStream(builder, iss, &root, &errs)) {
            Logger::Error("LoxoneAPI", "Failed to parse status response: " + errs);
            return "";
        }
        if (root.isMember("LL") && root["LL"].isObject() && root["LL"].isMember("value"))
            return root["LL"]["value"].asString();
    }
    catch (const std::exception& ex) {
        Logger::Error("LoxoneAPI", "Exception while parsing status value: " + std::string(ex.what()));
    }
    return "";
}

bool LoxoneAPI::ExtractStatusCode(const std::string& responseContent) {
    try {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errs;
        std::istringstream iss(responseContent);
        if (!Json::parseFromStream(builder, iss, &root, &errs)) {
            Logger::Error("LoxoneAPI", "Failed to parse Loxone response code: " + errs);
            return false;
        }
        if (root.isMember("LL") && root["LL"].isObject() && root["LL"].isMember("Code")) {
            std::string codeStr = root["LL"]["Code"].asString();
            return codeStr == "200";
        }
    }
    catch (const std::exception& ex) {
        Logger::Error("LoxoneAPI", "Exception while parsing response code: " + std::string(ex.what()));
    }
    return false;
}

bool LoxoneAPI::HttpGet(const std::string& url, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        Logger::Error("LoxoneAPI", "Failed to initialize cURL.");
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Basic " + Base64Encode(Config::LoxoneUser + ":" + Config::LoxonePW);
    headers = curl_slist_append(headers, authHeader.c_str());
    if (!headers) {
        Logger::Error("LoxoneAPI", "Failed to create HTTP headers.");
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        Logger::Error("LoxoneAPI", "HTTP GET failed for " + RedactUrl(url) + ": " + std::string(curl_easy_strerror(res)));
        return false;
    }

    if (httpCode != 200) {
        Logger::Error("LoxoneAPI", "HTTP status " + std::to_string(httpCode) + " for " + RedactUrl(url));
        return false;
    }

    return true;
}

std::string LoxoneAPI::Base64Encode(const std::string& in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

size_t LoxoneAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    if (userp)
    {
        std::string* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), totalSize);
    }
    return totalSize;
}
#include "civetweb.h"

int request_handler(struct mg_connection* conn, void* cbdata) {
    (void)cbdata;
    const mg_request_info* req_info = mg_get_request_info(conn);

    const std::string method = req_info->request_method ? req_info->request_method : "";
    const std::string client_ip = std::string(req_info->remote_addr);
    const std::string local_uri = req_info->local_uri ? req_info->local_uri : "";

    Logger::Info("HttpServer", "Incoming request method=" + method + " uri=" + local_uri + " client=" + client_ip);

    if (client_ip != Config::LoxoneIP) {
        Logger::Warn("HttpServer", "Rejected request because client IP does not match configured Loxone IP. expected=" + Config::LoxoneIP + " got=" + client_ip);
        mg_printf(conn,
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 15\r\n\r\n"
            "Forbidden: IP mismatch");
        return 1;
    }

    std::string query;

    if (method == "POST") {
        char buffer[1024];
        int bytes_read = mg_read(conn, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            query = buffer;
        }
        else {
            Logger::Warn("HttpServer", "POST body could not be read.");
        }
    }
    else if (method == "GET") {
        if (req_info->query_string) {
            query = req_info->query_string;
        }
        else {
            Logger::Warn("HttpServer", "GET request arrived without query string.");
        }
    }

    if (local_uri == "/set") {
        if (query.empty()) {
            Logger::Warn("HttpServer", "Rejected /set request because no query data was provided.");
            mg_printf(conn,
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 11\r\n\r\n"
                "Bad Request");
            return 1;
        }

        char valueBuffer[100] = { 0 };
        mg_get_var(query.c_str(), query.length(), "value", valueBuffer, sizeof(valueBuffer) - 1);

        std::string valueStr(valueBuffer);

        if (valueStr == "1") {
            Logger::Info("HttpServer", "Accepted /set request with value=1 from client=" + client_ip);
            LoxoneAPI loxoneAPI;
            loxoneAPI.CheckLoxoneValues();
        }
        else {
            Logger::Warn("HttpServer", "Unexpected value received on /set: " + valueStr);
        }

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 2\r\n\r\n"
            "OK");
        return 1;
    }

    Logger::Warn("HttpServer", "URI not found: " + local_uri);
    mg_printf(conn,
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 9\r\n\r\n"
        "Not Found");
    return 1;
}

#include <functional> // f\xfcr std::bind
bool LoxoneAPI::StartHttpServer() {
    if (httpContext != nullptr)
    {
        Logger::Warn("HttpServer", "StartHttpServer called while server is already running.");
        return true;
    }

    const std::string listenAddress = Config::HttpBindAddress + ":" + std::to_string(Config::HttpPort);
    const char* options[] = {
        "listening_ports", listenAddress.c_str(),
        nullptr
    };

    struct mg_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    httpContext = mg_start(&callbacks, nullptr, options);

    if (httpContext == nullptr) {
        Logger::Error("HttpServer", "Failed to start CivetWeb server on " + listenAddress + ".");
        httpServerRunning = false;
        return false;
    }

    mg_set_request_handler(httpContext, "/set", request_handler, nullptr);
    httpServerRunning = true;
    Logger::Info("HttpServer", "HTTP server is listening on " + listenAddress);
    Logger::Info("HttpServer", "Allowed client IP is " + Config::LoxoneIP);
    return true;
}

void LoxoneAPI::StopHttpServer()
{
    if (httpContext == nullptr)
    {
        httpServerRunning = false;
        return;
    }

    Logger::Info("HttpServer", "Stopping HTTP server.");
    mg_stop(httpContext);
    httpContext = nullptr;
    httpServerRunning = false;
    Logger::Info("HttpServer", "HTTP server stopped.");
}

bool LoxoneAPI::IsHttpServerRunning() const
{
    return httpServerRunning;
}



