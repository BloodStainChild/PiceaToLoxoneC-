#include "PiceaAPI.h"
#include "Config.h"
#include <curl/curl.h>
#include <json/json.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstdio>

// Initialisierung der statischen Mitglieder
std::function<void(PiceaData, PiceaSettingData)> PiceaAPI::OnDataFetched = nullptr;
bool PiceaAPI::isConnected = false;
bool PiceaAPI::isFatalError = false;

PiceaSettingData PiceaAPI::PSD = {};

bool PiceaAPI::StartLoop()
{
    while (!isFatalError)
    {
        if (TryToConnect())
        {
            while (isConnected)
            {
                FetchData();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return true;
}

bool PiceaAPI::TryToConnect()
{
    std::string apiUrl = "https://" + Config::PiceaIP + ":" + Config::PiceaPort + "/picea/v1/paired_devices?site_id=";
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Picea > Error initializing cURL." << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + Config::PiceaJWT;
    headers = curl_slist_append(headers, authHeader.c_str());
    if (!headers) {
        std::cerr << "Picea > Error creating headers." << std::endl;
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // SSL Peer-Verifizierung deaktivieren
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // SSL Host-Verifizierung deaktivieren

    std::string responseBody;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);


    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK)
    {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code == 200)
        {
            isConnected = true;
            std::cout << "Picea > Connection successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Picea > Connection failed: " << response_code << std::endl;
            isConnected = false;
        }
    }
    else
    {
        std::cerr << "Picea > Connection error: " << curl_easy_strerror(res) << std::endl;
        isConnected = false;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return isConnected;
}

bool PiceaAPI::FetchData()
{
    // Zusammenbau der Query (hier eine vereinfachte Version)
    std::string query = "autarky,"
        "battery_input_power,battery_output_power,battery_power,battery_state_of_charge,"
        "co2_avoidance,"
        "electrolyser_efficiency_electrical,electrolyser_efficiency_total,electrolyser_heating_power,electrolyser_output_power,"
        "extbattery_input_power,extbattery_output_power,extbattery_power,"
        "fuelcell_efficiency_electrical,fuelcell_efficiency_total,fuelcell_heating_power,fuelcell_input_power,"
        "grid_export_power,grid_import_power,grid_power,"
        "heat_contribution_power,hot_water_tempearture,"
        "house_to_picea_air_humidity,house_to_picea_air_temperature,household_input_power,household_input_power_from_battery_picea,household_input_power_from_extbattery,"
        "household_input_power_from_grid,household_input_power_from_hydrogen,household_input_power_from_picea,"
        "hydrogen_input_power_electrical,hydrogen_output_power_electrical,hydrogen_power_electrical,hydrogen_state_of_charge,"
        "is_efficiencymode,is_equal_charge_from_net,is_equal_charge_ongoing,is_grid_offline,is_heater_alarm,is_heater_error,"
        "is_hydrogen_used_except_reserve,is_in_ten_percent_grid_feedin_mode,is_launchphase,is_solar_error,"
        "is_ventilation_calibration_now,is_ventilation_differential_pressure_alarm,is_ventilation_filter_full_alarm,is_ventilation_filter_full_warning,is_water_error,"
        "max_compressor_blockage_duration,"
        "outdoor_to_picea_air_temperature,picea_to_house_air_temperature,"
        "solar_output_power,solar_output_power_ac,solar_output_power_dc_total,solar_output_power_to_battery,"
        "solar_output_power_to_extbattery,solar_output_power_to_grid,solar_output_power_to_household,solar_output_power_to_hydrogen,"
        "ventilation_stage_real";

    std::string apiUrl = "https://" + Config::PiceaIP + ":" + Config::PiceaPort + "/picea/v1/data/query/" + query + "?site_id=";
    std::string responseBody;

    if (!FetchFromApi(apiUrl, responseBody))
    {
        return false;
    }

    // Parsen der JSON-Antwort in PiceaData
    PiceaData pd = {};
    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;
    std::istringstream iss(responseBody);
    if (!Json::parseFromStream(builder, iss, &root, &errs))
    {
        std::cerr << "Picea > JSON Parsing Error: " << errs << std::endl;
        isConnected = false;
        return false;
    }
    // Autarky
    if (root.isMember("autarky") && root["autarky"].isMember("Ok") && root["autarky"]["Ok"].isMember("val")) {
        pd.autarky.ok.val = root["autarky"]["Ok"]["val"].asDouble();
    }

    // Battery Input Power
    if (root.isMember("battery_input_power") && root["battery_input_power"].isMember("Ok") && root["battery_input_power"]["Ok"].isMember("val")) {
        pd.battery_input_power.ok.val = root["battery_input_power"]["Ok"]["val"].asDouble();
    }

    // Battery Output Power
    if (root.isMember("battery_output_power") && root["battery_output_power"].isMember("Ok") && root["battery_output_power"]["Ok"].isMember("val")) {
        pd.battery_output_power.ok.val = root["battery_output_power"]["Ok"]["val"].asDouble();
    }

    // Battery Power
    if (root.isMember("battery_power") && root["battery_power"].isMember("Ok") && root["battery_power"]["Ok"].isMember("val")) {
        pd.battery_power.ok.val = root["battery_power"]["Ok"]["val"].asDouble();
    }

    // Battery State of Charge
    if (root.isMember("battery_state_of_charge") && root["battery_state_of_charge"].isMember("Ok") && root["battery_state_of_charge"]["Ok"].isMember("val")) {
        pd.battery_state_of_charge.ok.val = root["battery_state_of_charge"]["Ok"]["val"].asDouble();
    }

    // CO2 Avoidance
    if (root.isMember("co2_avoidance") && root["co2_avoidance"].isMember("Ok") && root["co2_avoidance"]["Ok"].isMember("val")) {
        pd.co2_avoidance.ok.val = root["co2_avoidance"]["Ok"]["val"].asDouble();
    }

    // Electrolyser Efficiency Electrical
    if (root.isMember("electrolyser_efficiency_electrical") && root["electrolyser_efficiency_electrical"].isMember("Ok") && root["electrolyser_efficiency_electrical"]["Ok"].isMember("val")) {
        pd.electrolyser_efficiency_electrical.ok.val = root["electrolyser_efficiency_electrical"]["Ok"]["val"].asDouble();
    }

    // Electrolyser Efficiency Total
    if (root.isMember("electrolyser_efficiency_total") && root["electrolyser_efficiency_total"].isMember("Ok") && root["electrolyser_efficiency_total"]["Ok"].isMember("val")) {
        pd.electrolyser_efficiency_total.ok.val = root["electrolyser_efficiency_total"]["Ok"]["val"].asDouble();
    }

    // Electrolyser Heating Power
    if (root.isMember("electrolyser_heating_power") && root["electrolyser_heating_power"].isMember("Ok") && root["electrolyser_heating_power"]["Ok"].isMember("val")) {
        pd.electrolyser_heating_power.ok.val = root["electrolyser_heating_power"]["Ok"]["val"].asDouble();
    }

    // Electrolyser Output Power
    if (root.isMember("electrolyser_output_power") && root["electrolyser_output_power"].isMember("Ok") && root["electrolyser_output_power"]["Ok"].isMember("val")) {
        pd.electrolyser_output_power.ok.val = root["electrolyser_output_power"]["Ok"]["val"].asDouble();
    }

    // External Battery Input Power
    if (root.isMember("extbattery_input_power") && root["extbattery_input_power"].isMember("Ok") && root["extbattery_input_power"]["Ok"].isMember("val")) {
        pd.extbattery_input_power.ok.val = root["extbattery_input_power"]["Ok"]["val"].asDouble();
    }

    // External Battery Output Power
    if (root.isMember("extbattery_output_power") && root["extbattery_output_power"].isMember("Ok") && root["extbattery_output_power"]["Ok"].isMember("val")) {
        pd.extbattery_output_power.ok.val = root["extbattery_output_power"]["Ok"]["val"].asDouble();
    }

    // External Battery Power
    if (root.isMember("extbattery_power") && root["extbattery_power"].isMember("Ok") && root["extbattery_power"]["Ok"].isMember("val")) {
        pd.extbattery_power.ok.val = root["extbattery_power"]["Ok"]["val"].asDouble();
    }

    // Fuelcell Efficiency Electrical
    if (root.isMember("fuelcell_efficiency_electrical") && root["fuelcell_efficiency_electrical"].isMember("Ok") && root["fuelcell_efficiency_electrical"]["Ok"].isMember("val")) {
        pd.fuelcell_efficiency_electrical.ok.val = root["fuelcell_efficiency_electrical"]["Ok"]["val"].asDouble();
    }

    // Fuelcell Efficiency Total
    if (root.isMember("fuelcell_efficiency_total") && root["fuelcell_efficiency_total"].isMember("Ok") && root["fuelcell_efficiency_total"]["Ok"].isMember("val")) {
        pd.fuelcell_efficiency_total.ok.val = root["fuelcell_efficiency_total"]["Ok"]["val"].asDouble();
    }

    // Fuelcell Heating Power
    if (root.isMember("fuelcell_heating_power") && root["fuelcell_heating_power"].isMember("Ok") && root["fuelcell_heating_power"]["Ok"].isMember("val")) {
        pd.fuelcell_heating_power.ok.val = root["fuelcell_heating_power"]["Ok"]["val"].asDouble();
    }

    // Fuelcell Input Power
    if (root.isMember("fuelcell_input_power") && root["fuelcell_input_power"].isMember("Ok") && root["fuelcell_input_power"]["Ok"].isMember("val")) {
        pd.fuelcell_input_power.ok.val = root["fuelcell_input_power"]["Ok"]["val"].asDouble();
    }

    // Grid Export Power
    if (root.isMember("grid_export_power") && root["grid_export_power"].isMember("Ok") && root["grid_export_power"]["Ok"].isMember("val")) {
        pd.grid_export_power.ok.val = root["grid_export_power"]["Ok"]["val"].asDouble();
    }

    // Grid Import Power
    if (root.isMember("grid_import_power") && root["grid_import_power"].isMember("Ok") && root["grid_import_power"]["Ok"].isMember("val")) {
        pd.grid_import_power.ok.val = root["grid_import_power"]["Ok"]["val"].asDouble();
    }

    // Grid Power
    if (root.isMember("grid_power") && root["grid_power"].isMember("Ok") && root["grid_power"]["Ok"].isMember("val")) {
        pd.grid_power.ok.val = root["grid_power"]["Ok"]["val"].asDouble();
    }

    // Heat Contribution Power
    if (root.isMember("heat_contribution_power") && root["heat_contribution_power"].isMember("Ok") && root["heat_contribution_power"]["Ok"].isMember("val")) {
        pd.heat_contribution_power.ok.val = root["heat_contribution_power"]["Ok"]["val"].asDouble();
    }

    // Hot Water Temperature
    if (root.isMember("hot_water_tempearture") && root["hot_water_tempearture"].isMember("Ok") && root["hot_water_tempearture"]["Ok"].isMember("val")) {
        pd.hot_water_tempearture.ok.val = root["hot_water_tempearture"]["Ok"]["val"].asDouble();
    }

    // House to Picea Air Humidity
    if (root.isMember("house_to_picea_air_humidity") && root["house_to_picea_air_humidity"].isMember("Ok") && root["house_to_picea_air_humidity"]["Ok"].isMember("val")) {
        pd.house_to_picea_air_humidity.ok.val = root["house_to_picea_air_humidity"]["Ok"]["val"].asDouble();
    }

    // House to Picea Air Temperature
    if (root.isMember("house_to_picea_air_temperature") && root["house_to_picea_air_temperature"].isMember("Ok") && root["house_to_picea_air_temperature"]["Ok"].isMember("val")) {
        pd.house_to_picea_air_temperature.ok.val = root["house_to_picea_air_temperature"]["Ok"]["val"].asDouble();
    }

    // Household Input Power
    if (root.isMember("household_input_power") && root["household_input_power"].isMember("Ok") && root["household_input_power"]["Ok"].isMember("val")) {
        pd.household_input_power.ok.val = root["household_input_power"]["Ok"]["val"].asDouble();
    }

    // Household Input Power from Battery
    if (root.isMember("household_input_power_from_battery_picea") && root["household_input_power_from_battery_picea"].isMember("Ok") && root["household_input_power_from_battery_picea"]["Ok"].isMember("val")) {
        pd.household_input_power_from_battery_picea.ok.val = root["household_input_power_from_battery_picea"]["Ok"]["val"].asDouble();
    }

    // Household Input Power from External Battery
    if (root.isMember("household_input_power_from_extbattery") && root["household_input_power_from_extbattery"].isMember("Ok") && root["household_input_power_from_extbattery"]["Ok"].isMember("val")) {
        pd.household_input_power_from_extbattery.ok.val = root["household_input_power_from_extbattery"]["Ok"]["val"].asDouble();
    }

    // Household Input Power from Grid
    if (root.isMember("household_input_power_from_grid") && root["household_input_power_from_grid"].isMember("Ok") && root["household_input_power_from_grid"]["Ok"].isMember("val")) {
        pd.household_input_power_from_grid.ok.val = root["household_input_power_from_grid"]["Ok"]["val"].asDouble();
    }

    // Household Input Power from Hydrogen
    if (root.isMember("household_input_power_from_hydrogen") && root["household_input_power_from_hydrogen"].isMember("Ok") && root["household_input_power_from_hydrogen"]["Ok"].isMember("val")) {
        pd.household_input_power_from_hydrogen.ok.val = root["household_input_power_from_hydrogen"]["Ok"]["val"].asDouble();
    }

    // Household Input Power from Picea
    if (root.isMember("household_input_power_from_picea") && root["household_input_power_from_picea"].isMember("Ok") && root["household_input_power_from_picea"]["Ok"].isMember("val")) {
        pd.household_input_power_from_picea.ok.val = root["household_input_power_from_picea"]["Ok"]["val"].asDouble();
    }

    // Hydrogen Production Power
    if (root.isMember("hydrogen_input_power_electrical") && root["hydrogen_input_power_electrical"].isMember("Ok") && root["hydrogen_input_power_electrical"]["Ok"].isMember("val")) {
        pd.hydrogen_input_power_electrical.ok.val = root["hydrogen_input_power_electrical"]["Ok"]["val"].asDouble();
    }

    // Hydrogen Production Power
    if (root.isMember("hydrogen_output_power_electrical") && root["hydrogen_output_power_electrical"].isMember("Ok") && root["hydrogen_output_power_electrical"]["Ok"].isMember("val")) {
        pd.hydrogen_output_power_electrical.ok.val = root["hydrogen_output_power_electrical"]["Ok"]["val"].asDouble();
    }

    // Hydrogen Power electrical
    if (root.isMember("hydrogen_power_electrical") && root["hydrogen_power_electrical"].isMember("Ok") && root["hydrogen_power_electrical"]["Ok"].isMember("val")) {
        pd.hydrogen_power_electrical.ok.val = root["hydrogen_power_electrical"]["Ok"]["val"].asDouble();
    }

    // Hydrogen state of charge
    if (root.isMember("hydrogen_state_of_charge") && root["hydrogen_state_of_charge"].isMember("Ok") && root["hydrogen_state_of_charge"]["Ok"].isMember("val")) {
        pd.hydrogen_state_of_charge.ok.val = root["hydrogen_state_of_charge"]["Ok"]["val"].asDouble();
    }

    // Is Efficiency Mode
    if (root.isMember("is_efficiencymode") && root["is_efficiencymode"].isMember("Ok") && root["is_efficiencymode"]["Ok"].isMember("val")) {
        pd.is_efficiencymode.ok.val = root["is_efficiencymode"]["Ok"]["val"].asBool();
    }

    // Is Equal Charge from Net
    if (root.isMember("is_equal_charge_from_net") && root["is_equal_charge_from_net"].isMember("Ok") && root["is_equal_charge_from_net"]["Ok"].isMember("val")) {
        pd.is_equal_charge_from_net.ok.val = root["is_equal_charge_from_net"]["Ok"]["val"].asBool();
    }

    // Is Equal Charge Ongoing
    if (root.isMember("is_equal_charge_ongoing") && root["is_equal_charge_ongoing"].isMember("Ok") && root["is_equal_charge_ongoing"]["Ok"].isMember("val")) {
        pd.is_equal_charge_ongoing.ok.val = root["is_equal_charge_ongoing"]["Ok"]["val"].asBool();
    }

    // Is Grid Offline
    if (root.isMember("is_grid_offline") && root["is_grid_offline"].isMember("Ok") && root["is_grid_offline"]["Ok"].isMember("val")) {
        pd.is_grid_offline.ok.val = root["is_grid_offline"]["Ok"]["val"].asBool();
    }
    // Is Heater Alarm
    if (root.isMember("is_heater_alarm") && root["is_heater_alarm"].isMember("Ok") && root["is_heater_alarm"]["Ok"].isMember("val")) {
        pd.is_heater_alarm.ok.val = root["is_heater_alarm"]["Ok"]["val"].asBool();
    }

    // Is Heater Error
    if (root.isMember("is_heater_error") && root["is_heater_error"].isMember("Ok") && root["is_heater_error"]["Ok"].isMember("val")) {
        pd.is_heater_error.ok.val = root["is_heater_error"]["Ok"]["val"].asBool();
    }

    // Is Hydrogen Used Except Reserve
    if (root.isMember("is_hydrogen_used_except_reserve") && root["is_hydrogen_used_except_reserve"].isMember("Ok") && root["is_hydrogen_used_except_reserve"]["Ok"].isMember("val")) {
        pd.is_hydrogen_used_except_reserve.ok.val = root["is_hydrogen_used_except_reserve"]["Ok"]["val"].asBool();
    }

    // Is In Ten Percent Grid Feedin Mode
    if (root.isMember("is_in_ten_percent_grid_feedin_mode") && root["is_in_ten_percent_grid_feedin_mode"].isMember("Ok") && root["is_in_ten_percent_grid_feedin_mode"]["Ok"].isMember("val")) {
        pd.is_in_ten_percent_grid_feedin_mode.ok.val = root["is_in_ten_percent_grid_feedin_mode"]["Ok"]["val"].asBool();
    }

    // Is Launch Phase
    if (root.isMember("is_launchphase") && root["is_launchphase"].isMember("Ok") && root["is_launchphase"]["Ok"].isMember("val")) {
        pd.is_launchphase.ok.val = root["is_launchphase"]["Ok"]["val"].asBool();
    }

    // Is Solar Error
    if (root.isMember("is_solar_error") && root["is_solar_error"].isMember("Ok") && root["is_solar_error"]["Ok"].isMember("val")) {
        pd.is_solar_error.ok.val = root["is_solar_error"]["Ok"]["val"].asBool();
    }

    // Is Ventilation Calibration Now
    if (root.isMember("is_ventilation_calibration_now") && root["is_ventilation_calibration_now"].isMember("Ok") && root["is_ventilation_calibration_now"]["Ok"].isMember("val")) {
        pd.is_ventilation_calibration_now.ok.val = root["is_ventilation_calibration_now"]["Ok"]["val"].asBool();
    }

    // Is Ventilation Differential Pressure Alarm
    if (root.isMember("is_ventilation_differential_pressure_alarm") && root["is_ventilation_differential_pressure_alarm"].isMember("Ok") && root["is_ventilation_differential_pressure_alarm"]["Ok"].isMember("val")) {
        pd.is_ventilation_differential_pressure_alarm.ok.val = root["is_ventilation_differential_pressure_alarm"]["Ok"]["val"].asBool();
    }

    // Is Ventilation Filter Full Alarm
    if (root.isMember("is_ventilation_filter_full_alarm") && root["is_ventilation_filter_full_alarm"].isMember("Ok") && root["is_ventilation_filter_full_alarm"]["Ok"].isMember("val")) {
        pd.is_ventilation_filter_full_alarm.ok.val = root["is_ventilation_filter_full_alarm"]["Ok"]["val"].asBool();
    }

    // Is Ventilation Filter Full Warning
    if (root.isMember("is_ventilation_filter_full_warning") && root["is_ventilation_filter_full_warning"].isMember("Ok") && root["is_ventilation_filter_full_warning"]["Ok"].isMember("val")) {
        pd.is_ventilation_filter_full_warning.ok.val = root["is_ventilation_filter_full_warning"]["Ok"]["val"].asBool();
    }

    // Is Water Error
    if (root.isMember("is_water_error") && root["is_water_error"].isMember("Ok") && root["is_water_error"]["Ok"].isMember("val")) {
        pd.is_water_error.ok.val = root["is_water_error"]["Ok"]["val"].asBool();
    }

    // Max Compressor Blockage Duration
    if (root.isMember("max_compressor_blockage_duration") && root["max_compressor_blockage_duration"].isMember("Ok") && root["max_compressor_blockage_duration"]["Ok"].isMember("val")) {
        pd.max_compressor_blockage_duration.ok.val = root["max_compressor_blockage_duration"]["Ok"]["val"].asInt();
    }

    // Outdoor to Picea Air Temperature
    if (root.isMember("outdoor_to_picea_air_temperature") && root["outdoor_to_picea_air_temperature"].isMember("Ok") && root["outdoor_to_picea_air_temperature"]["Ok"].isMember("val")) {
        pd.outdoor_to_picea_air_temperature.ok.val = root["outdoor_to_picea_air_temperature"]["Ok"]["val"].asDouble();
    }

    // Picea to House Air Temperature
    if (root.isMember("picea_to_house_air_temperature") && root["picea_to_house_air_temperature"].isMember("Ok") && root["picea_to_house_air_temperature"]["Ok"].isMember("val")) {
        pd.picea_to_house_air_temperature.ok.val = root["picea_to_house_air_temperature"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power
    if (root.isMember("solar_output_power") && root["solar_output_power"].isMember("Ok") && root["solar_output_power"]["Ok"].isMember("val")) {
        pd.solar_output_power.ok.val = root["solar_output_power"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power AC
    if (root.isMember("solar_output_power_ac") && root["solar_output_power_ac"].isMember("Ok") && root["solar_output_power_ac"]["Ok"].isMember("val")) {
        pd.solar_output_power_ac.ok.val = root["solar_output_power_ac"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power DC Total
    if (root.isMember("solar_output_power_dc_total") && root["solar_output_power_dc_total"].isMember("Ok") && root["solar_output_power_dc_total"]["Ok"].isMember("val")) {
        pd.solar_output_power_dc_total.ok.val = root["solar_output_power_dc_total"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power to Battery
    if (root.isMember("solar_output_power_to_battery") && root["solar_output_power_to_battery"].isMember("Ok") && root["solar_output_power_to_battery"]["Ok"].isMember("val")) {
        pd.solar_output_power_to_battery.ok.val = root["solar_output_power_to_battery"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power to External Battery
    if (root.isMember("solar_output_power_to_extbattery") && root["solar_output_power_to_extbattery"].isMember("Ok") && root["solar_output_power_to_extbattery"]["Ok"].isMember("val")) {
        pd.solar_output_power_to_extbattery.ok.val = root["solar_output_power_to_extbattery"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power to Grid
    if (root.isMember("solar_output_power_to_grid") && root["solar_output_power_to_grid"].isMember("Ok") && root["solar_output_power_to_grid"]["Ok"].isMember("val")) {
        pd.solar_output_power_to_grid.ok.val = root["solar_output_power_to_grid"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power to Household
    if (root.isMember("solar_output_power_to_household") && root["solar_output_power_to_household"].isMember("Ok") && root["solar_output_power_to_household"]["Ok"].isMember("val")) {
        pd.solar_output_power_to_household.ok.val = root["solar_output_power_to_household"]["Ok"]["val"].asDouble();
    }

    // Solar Output Power to Hydrogen
    if (root.isMember("solar_output_power_to_hydrogen") && root["solar_output_power_to_hydrogen"].isMember("Ok") && root["solar_output_power_to_hydrogen"]["Ok"].isMember("val")) {
        pd.solar_output_power_to_hydrogen.ok.val = root["solar_output_power_to_hydrogen"]["Ok"]["val"].asDouble();
    }

    // Ventilation Stage Real
    if (root.isMember("ventilation_stage_real") && root["ventilation_stage_real"].isMember("Ok") && root["ventilation_stage_real"]["Ok"].isMember("val")) {
        pd.ventilation_stage_real.ok.val = root["ventilation_stage_real"]["Ok"]["val"].asInt();
    }


    // Konfiguration abrufen
    apiUrl = "https://" + Config::PiceaIP + ":" + Config::PiceaPort + "/picea/v1/config?site_id=";
    responseBody = "";
    if (!FetchFromApi(apiUrl, responseBody))
    {
        return false;
    }

    PiceaSettingData psd = {};
    std::istringstream iss2(responseBody);
    Json::Value root2;
    if (!Json::parseFromStream(builder, iss2, &root2, &errs))
    {
        std::cerr << "Picea > JSON Parsing Error (config): " << errs << std::endl;
        isConnected = false;
        return false;
    }

    // Zuweisung der verschachtelten Werte aus dem JSON
    if (root2.isMember("compressor_blockage_duration") && root2["compressor_blockage_duration"].isMember("value")) {
        psd.compressor_blockage_duration.value = root2["compressor_blockage_duration"]["value"].isNull() ? 0.0 : root2["compressor_blockage_duration"]["value"].asDouble();
    }

    if (root2.isMember("ext_battery_setup") && root2["ext_battery_setup"].isMember("value")) {
        std::string extBatteryValue = root2["ext_battery_setup"]["value"].asString();
        // Setze den Wert je nach den möglichen Optionen
        if (extBatteryValue == "none") {
            psd.ext_battery_setup.setValue(ExtBatterySetup::none);
        }
        else if (extBatteryValue == "Ext_basic") {
            psd.ext_battery_setup.setValue(ExtBatterySetup::Ext_basic);
        }
        else if (extBatteryValue == "Ext_hybrid") {
            psd.ext_battery_setup.setValue(ExtBatterySetup::Ext_hybrid);
        }
    }

    if (root2.isMember("hydrogen_reserve") && root2["hydrogen_reserve"].isMember("value")) {
        psd.hydrogen_reserve.value = root2["hydrogen_reserve"]["value"].asDouble();
    }

    if (root2.isMember("filter_exchange_state") && root2["filter_exchange_state"].isMember("value")) {
        std::string Value = root2["filter_exchange_state"]["value"].asString();

        if (Value == "init") {
            psd.filter_exchange_state.setValue(FilterExchangeState::init);
        }
        else if (Value == "filterchange_currently_possible") {
            psd.filter_exchange_state.setValue(FilterExchangeState::filterchange_currently_possible);
        }
        else if (Value == "filterchange_preparing") {
            psd.filter_exchange_state.setValue(FilterExchangeState::filterchange_preparing);
        }
        else if (Value == "ready_for_filterchange") {
            psd.filter_exchange_state.setValue(FilterExchangeState::ready_for_filterchange);
        }
        else if (Value == "filterchange_verifying") {
            psd.filter_exchange_state.setValue(FilterExchangeState::filterchange_verifying);
        }
        else if (Value == "filterchange_verification_failed") {
            psd.filter_exchange_state.setValue(FilterExchangeState::filterchange_verification_failed);
        }

        else if (Value == "filterchange_currently_NOT_possible") {
            psd.filter_exchange_state.setValue(FilterExchangeState::filterchange_currently_NOT_possible);
        }
    }

    if (root2.isMember("ventilation_night_time_start") && root2["ventilation_night_time_start"].isMember("value")) {
        psd.ventilation_night_time_start.value = root2["ventilation_night_time_start"]["value"].asString();
    }

    if (root2.isMember("has_grid_tax_feedin") && root2["has_grid_tax_feedin"].isMember("value")) {
        psd.has_grid_tax_feedin.value = root2["has_grid_tax_feedin"]["value"].asBool();
    }

    if (root2.isMember("is_surpluspower_heatpump_enabled") && root2["is_surpluspower_heatpump_enabled"].isMember("value")) {
        psd.is_surpluspower_heatpump_enabled.value = root2["is_surpluspower_heatpump_enabled"]["value"].asBool();
    }

    if (root2.isMember("has_solar_ac") && root2["has_solar_ac"].isMember("value")) {
        psd.has_solar_ac.value = root2["has_solar_ac"]["value"].asBool();
    }

    if (root2.isMember("has_no_hot_water_integrated") && root2["has_no_hot_water_integrated"].isMember("value")) {
        psd.has_no_hot_water_integrated.value = root2["has_no_hot_water_integrated"]["value"].asBool();
    }

    if (root2.isMember("ventilation_stage_user") && root2["ventilation_stage_user"].isMember("value")) {
        psd.ventilation_stage_user.value = root2["ventilation_stage_user"]["value"].asInt();
    }

    if (root2.isMember("has_solar_ac_and_dc") && root2["has_solar_ac_and_dc"].isMember("value")) {
        psd.has_solar_ac_and_dc.value = root2["has_solar_ac_and_dc"]["value"].asBool();
    }

    if (root2.isMember("is_surpluspower_immersionheater_enabled") && root2["is_surpluspower_immersionheater_enabled"].isMember("value")) {
        psd.is_surpluspower_immersionheater_enabled.value = root2["is_surpluspower_immersionheater_enabled"]["value"].asBool();
    }

    if (root2.isMember("has_differential_pressure_gauge") && root2["has_differential_pressure_gauge"].isMember("value")) {
        psd.has_differential_pressure_gauge.value = root2["has_differential_pressure_gauge"]["value"].asBool();
    }

    if (root2.isMember("party_mode_enabled") && root2["party_mode_enabled"].isMember("value")) {
        psd.party_mode_enabled.value = root2["party_mode_enabled"]["value"].asBool();
    }

    if (root2.isMember("picea_generation") && root2["picea_generation"].isMember("value")) {
        std::string Value = root2["picea_generation"]["value"].asString();
        if (Value == "P1") {
            psd.picea_generation.setValue(PiceaGeneration::P1);
        }
        else if (Value == "P2") {
            psd.picea_generation.setValue(PiceaGeneration::P2);
        }
        else if (Value == "P3") {
            psd.picea_generation.setValue(PiceaGeneration::P3);
        }
    }

    if (root2.isMember("picea_serial") && root2["picea_serial"].isMember("value")) {
        psd.picea_serial.value = root2["picea_serial"]["value"].asString();
    }

    if (root2.isMember("has_immersion_heater") && root2["has_immersion_heater"].isMember("value")) {
        psd.has_immersion_heater.value = root2["has_immersion_heater"]["value"].asBool();
    }

    if (root2.isMember("is_grid_connected_system") && root2["is_grid_connected_system"].isMember("value")) {
        psd.is_grid_connected_system.value = root2["is_grid_connected_system"]["value"].asBool();
    }

    if (root2.isMember("ventilation_night_stage") && root2["ventilation_night_stage"].isMember("value")) {
        psd.ventilation_night_stage.value = root2["ventilation_night_stage"]["value"].asInt();
    }

    if (root2.isMember("is_surpluspower_grid_export_enabled") && root2["is_surpluspower_grid_export_enabled"].isMember("value")) {
        psd.is_surpluspower_grid_export_enabled.value = root2["is_surpluspower_grid_export_enabled"]["value"].asBool();
    }

    if (root2.isMember("picea_count") && root2["picea_count"].isMember("value")) {
        psd.picea_count.value = root2["picea_count"]["value"].asDouble();
    }

    if (root2.isMember("ventilation_night_enabled") && root2["ventilation_night_enabled"].isMember("value")) {
        psd.ventilation_night_enabled.value = root2["ventilation_night_enabled"]["value"].asBool();
    }

    if (root2.isMember("ventilation_temperature_target") && root2["ventilation_temperature_target"].isMember("value")) {
        psd.ventilation_temperature_target.value = root2["ventilation_temperature_target"]["value"].asDouble();
    }

    if (root2.isMember("has_sgready") && root2["has_sgready"].isMember("value")) {
        psd.has_sgready.value = root2["has_sgready"]["value"].asBool();
    }

    if (root2.isMember("has_solar_dc") && root2["has_solar_dc"].isMember("value")) {
        psd.has_solar_dc.value = root2["has_solar_dc"]["value"].asBool();
    }

    if (root2.isMember("ventilation_night_time_end") && root2["ventilation_night_time_end"].isMember("value")) {
        psd.ventilation_night_time_end.value = root2["ventilation_night_time_end"]["value"].asString();
    }

    if (root2.isMember("has_no_heat_contribution") && root2["has_no_heat_contribution"].isMember("value")) {
        psd.has_no_heat_contribution.value = root2["has_no_heat_contribution"]["value"].asBool();
    }

    if (root2.isMember("has_ventilation_integrated") && root2["has_ventilation_integrated"].isMember("value")) {
        psd.has_ventilation_integrated.value = root2["has_ventilation_integrated"]["value"].asBool();
    }

    if (root2.isMember("vacation_mode_enabled") && root2["vacation_mode_enabled"].isMember("value")) {
        psd.vacation_mode_enabled.value = root2["vacation_mode_enabled"]["value"].asBool();
    }
    PSD = psd;

    if (OnDataFetched)
    {
        OnDataFetched(pd, psd);
    }
    return true;
}

bool PiceaAPI::FetchFromApi(const std::string& apiUrl, std::string& responseBody)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Picea > Error initializing cURL." << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + Config::PiceaJWT;
    headers = curl_slist_append(headers, authHeader.c_str());
    if (!headers) {
        std::cerr << "Picea > Error creating headers." << std::endl;
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // SSL Peer-Verifizierung deaktivieren
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // SSL Host-Verifizierung deaktivieren

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "Picea > HTTP Error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return true;
}

bool PiceaAPI::UpdateConfigSettings(const std::string& jsonData)
{
    if (!isConnected) return false;

    std::string apiUrl = "https://" + Config::PiceaIP + ":" + Config::PiceaPort + "/picea/v1/config?site_id=";

    // Überprüfe, ob apiUrl korrekt formatiert ist
    if (apiUrl.find("https://") != 0 || Config::PiceaIP.empty() || Config::PiceaPort.empty()) {
        std::cerr << "Picea > Invalid URL: " << apiUrl << std::endl;
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Picea > Error initializing cURL." << std::endl;
        return false;
    }

    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + Config::PiceaJWT;
    headers = curl_slist_append(headers, authHeader.c_str());
    if (!headers) {
        std::cerr << "Picea > Error creating headers." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!headers) {
        std::cerr << "Picea > Error creating headers." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // SSL-Überprüfung deaktivieren (nur für Testumgebungen!)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // SSL Peer-Verifizierung deaktivieren
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // SSL Host-Verifizierung deaktivieren

    // Explizit POST-Methode setzen
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

    // URL setzen
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

    // Write Callback und WRITEDATA setzen, um die Serverantwort zu sammeln
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string responseBody;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "Picea > Error updating settings: " << curl_easy_strerror(res) << std::endl;
        std::cerr << "Response: " << responseBody << std::endl;
    }

    // Ressourcen freigeben
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

size_t PiceaAPI::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    if (userp)
    {
        std::string* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), totalSize);
    }
    return totalSize;
}                                  

void PiceaAPI::SendSettingsData(const PiceaSettingData& settings)
{
    // Konvertiere Einstellungen in JSON mithilfe von jsoncpp
    Json::Value root;

    if (settings.party_mode_enabled.value.has_value())
    {
        root["party_mode_enabled"]["value"] = settings.party_mode_enabled.value.value();
    }
    if (settings.vacation_mode_enabled.value.has_value())
    {
        root["vacation_mode_enabled"]["value"] = settings.vacation_mode_enabled.value.value();
    }
    if (settings.has_grid_tax_feedin.value.has_value())
    {
        root["has_grid_tax_feedin"]["value"] = settings.has_grid_tax_feedin.value.value();
    }
    if (settings.is_grid_connected_system.value.has_value())
    {
        root["is_grid_connected_system"]["value"] = settings.is_grid_connected_system.value.value();
    }
    if (settings.has_no_hot_water_integrated.value.has_value())
    {
        root["has_no_hot_water_integrated"]["value"] = settings.has_no_hot_water_integrated.value.value();
    }
    if (settings.has_differential_pressure_gauge.value.has_value())
    {
        root["has_differential_pressure_gauge"]["value"] = settings.has_differential_pressure_gauge.value.value();
    }
    if (settings.hydrogen_reserve.value.has_value())
    {
        root["hydrogen_reserve"]["value"] = settings.hydrogen_reserve.value.value();
    }
    if (settings.ext_battery_setup.value.has_value())
    {
        ExtBatterySetup val = settings.ext_battery_setup.getValue();
        switch (val)
        {
        case ExtBatterySetup::none:
            root["ext_battery_setup"]["value"] = "none";
            break;
        case ExtBatterySetup::Ext_basic:
            root["ext_battery_setup"]["value"] = "Ext_basic";
            break;
        case ExtBatterySetup::Ext_hybrid:
            root["ext_battery_setup"]["value"] = "Ext_hybrid";
            break;
        default:
            root["ext_battery_setup"]["value"] = "none";
            break;
        }
    }
    if (settings.filter_exchange_state.value.has_value())
    {
        FilterExchangeState val = settings.filter_exchange_state.getValue();
        switch (val)
        {
        case FilterExchangeState::init:
            root["filter_exchange_state"]["value"] = "init";
            break;
        case FilterExchangeState::filterchange_currently_possible:
            root["filter_exchange_state"]["value"] = "filterchange_currently_possible";
            break;
        case FilterExchangeState::filterchange_preparing:
            root["filter_exchange_state"]["value"] = "filterchange_preparing";
            break;
        case FilterExchangeState::ready_for_filterchange:
            root["filter_exchange_state"]["value"] = "ready_for_filterchange";
            break;
        case FilterExchangeState::filterchange_verifying:
            root["filter_exchange_state"]["value"] = "filterchange_verifying";
            break;
        case FilterExchangeState::filterchange_verification_failed:
            root["filter_exchange_state"]["value"] = "filterchange_verification_failed";
            break;
        case FilterExchangeState::filterchange_currently_NOT_possible:
            root["filter_exchange_state"]["value"] = "filterchange_currently_NOT_possible";
            break;
        default:
            root["filter_exchange_state"]["value"] = "filterchange_currently_possible";
            break;
        }
    }
    // Ventilation
    if (settings.ventilation_stage_user.value.has_value())
    {
        root["ventilation_stage_user"]["value"] = settings.ventilation_stage_user.value.value();
    }
    if (settings.ventilation_temperature_target.value.has_value())
    {
        root["ventilation_temperature_target"]["value"] = settings.ventilation_temperature_target.value.value();
    }
    if (settings.ventilation_night_enabled.value.has_value())
    {
        root["ventilation_night_enabled"]["value"] = settings.ventilation_night_enabled.value.value();
    }
    if (settings.ventilation_night_stage.value.has_value())
    {
        root["ventilation_night_stage"]["value"] = settings.ventilation_night_stage.value.value();
    }
    if (settings.ventilation_night_time_start.value.has_value())
    {
        root["ventilation_night_time_start"]["value"] = settings.ventilation_night_time_start.getValue();
    }
    if (settings.ventilation_night_time_end.value.has_value())
    {
        root["ventilation_night_time_end"]["value"] = settings.ventilation_night_time_end.getValue();
    }
    //
    if (settings.compressor_blockage_duration.value.has_value())
    {
        root["compressor_blockage_duration"]["value"] = settings.compressor_blockage_duration.value.value();
    }
    if (settings.is_surpluspower_heatpump_enabled.value.has_value())
    {
        root["is_surpluspower_heatpump_enabled"]["value"] = settings.is_surpluspower_heatpump_enabled.value.value();
    }
    if (settings.is_surpluspower_grid_export_enabled.value.has_value())
    {
        root["is_surpluspower_grid_export_enabled"]["value"] = settings.is_surpluspower_grid_export_enabled.value.value();
    }
    if (settings.is_surpluspower_immersionheater_enabled.value.has_value())
    {
        root["is_surpluspower_immersionheater_enabled"]["value"] = settings.is_surpluspower_immersionheater_enabled.value.value();
    }

    Json::StreamWriterBuilder writer;
    std::string jsonData = Json::writeString(writer, root);

    // Sende die aktualisierten Einstellungen an die API
    UpdateConfigSettings(jsonData);
}

