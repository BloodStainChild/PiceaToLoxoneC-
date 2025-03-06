#include "Config.h"
#include <iostream>
#include <fstream>
#include <sstream>

const std::string Config::filePath = "config.cfg";
std::string Config::LoxoneIP = "";
std::string Config::LoxoneUser = "";
std::string Config::LoxonePW = "";
std::string Config::PiceaIP = "";
std::string Config::PiceaPort = "";
std::string Config::PiceaJWT = "";

std::string Config::autarky = "";
std::string Config::battery_input_power = "";
std::string Config::battery_output_power = "";
std::string Config::battery_power = "";
std::string Config::battery_state_of_charge = "";
std::string Config::co2_avoidance = "";
std::string Config::electrolyser_efficiency_electrical = "";
std::string Config::electrolyser_efficiency_total = "";
std::string Config::electrolyser_heating_power = "";
std::string Config::electrolyser_output_power = "";
std::string Config::extbattery_input_power = "";
std::string Config::extbattery_output_power = "";
std::string Config::extbattery_power = "";
std::string Config::fuelcell_efficiency_electrical = "";
std::string Config::fuelcell_efficiency_total = "";
std::string Config::fuelcell_heating_power = "";
std::string Config::fuelcell_input_power = "";
std::string Config::grid_export_power = "";
std::string Config::grid_import_power = "";
std::string Config::grid_power = "";
std::string Config::heat_contribution_power = "";
std::string Config::hot_water_tempearture = "";
std::string Config::house_to_picea_air_humidity = "";
std::string Config::house_to_picea_air_temperature = "";
std::string Config::household_input_power = "";
std::string Config::household_input_power_from_battery_picea = "";
std::string Config::household_input_power_from_extbattery = "";
std::string Config::household_input_power_from_grid = "";
std::string Config::household_input_power_from_hydrogen = "";
std::string Config::household_input_power_from_picea = "";
std::string Config::hydrogen_input_power_electrical = "";
std::string Config::hydrogen_output_power_electrical = "";
std::string Config::hydrogen_power_electrical = "";
std::string Config::hydrogen_state_of_charge = "";
std::string Config::is_efficiencymode = "";
std::string Config::is_equal_charge_from_net = "";
std::string Config::is_equal_charge_ongoing = "";
std::string Config::is_grid_offline = "";
std::string Config::is_heater_alarm = "";
std::string Config::is_heater_error = "";
std::string Config::is_hydrogen_used_except_reserve = "";
std::string Config::is_in_ten_percent_grid_feedin_mode = "";
std::string Config::is_launchphase = "";
std::string Config::is_solar_error = "";
std::string Config::is_ventilation_calibration_now = "";
std::string Config::is_ventilation_differential_pressure_alarm = "";
std::string Config::is_ventilation_filter_full_alarm = "";
std::string Config::is_ventilation_filter_full_warning = "";
std::string Config::is_water_error = "";
std::string Config::max_compressor_blockage_duration = "";
std::string Config::outdoor_to_picea_air_temperature = "";
std::string Config::picea_to_house_air_temperature;
std::string Config::solar_output_power = "";
std::string Config::solar_output_power_ac = "";
std::string Config::solar_output_power_dc_total = "";
std::string Config::solar_output_power_to_battery = "";
std::string Config::solar_output_power_to_extbattery = "";
std::string Config::solar_output_power_to_grid = "";
std::string Config::solar_output_power_to_household = "";
std::string Config::solar_output_power_to_hydrogen = "";
std::string Config::ventilation_stage_real = "";

std::string Config::party_mode_enabled_IN = "";
std::string Config::vacation_mode_enabled_IN = "";
std::string Config::has_grid_tax_feedin_IN = "";
std::string Config::is_grid_connected_system_IN = "";
std::string Config::has_no_hot_water_integrated_IN = "";
std::string Config::has_differential_pressure_gauge_IN = "";
std::string Config::hydrogen_reserve_IN = "";
std::string Config::ext_battery_setup_IN = "";
std::string Config::filter_exchange_state_IN = "";
std::string Config::ventilation_stage_user_IN = "";
std::string Config::ventilation_temperature_target_IN = "";
std::string Config::ventilation_night_enabled_IN = "";
std::string Config::ventilation_night_stage_IN = "";
std::string Config::ventilation_night_time_end_IN = "";
std::string Config::ventilation_night_time_start_IN = "";
std::string Config::compressor_blockage_duration_IN = "";
std::string Config::is_surpluspower_heatpump_enabled_IN = "";
std::string Config::is_surpluspower_grid_export_enabled_IN = "";
std::string Config::is_surpluspower_immersionheater_enabled_IN = "";
std::string Config::picea_generation_IN = "";
std::string Config::picea_count_IN = "";
std::string Config::picea_serial_IN = "";
std::string Config::has_solar_dc_IN = "";
std::string Config::has_solar_ac_IN = "";
std::string Config::has_solar_ac_and_dc_IN = "";
std::string Config::has_sgready_IN = "";
std::string Config::has_no_heat_contribution_IN = "";
std::string Config::has_immersion_heater_IN = "";
std::string Config::has_ventilation_integrated_IN = "";

std::string Config::party_mode_enabled_OUT = "";
std::string Config::vacation_mode_enabled_OUT = "";
std::string Config::has_grid_tax_feedin_OUT = "";
std::string Config::is_grid_connected_system_OUT = "";
std::string Config::has_no_hot_water_integrated_OUT = "";
std::string Config::has_differential_pressure_gauge_OUT = "";
std::string Config::hydrogen_reserve_OUT = "";
std::string Config::ext_battery_setup_OUT = "";
std::string Config::filter_exchange_state_OUT = "";
std::string Config::ventilation_stage_user_OUT = "";
std::string Config::ventilation_temperature_target_OUT = "";
std::string Config::ventilation_night_enabled_OUT = "";
std::string Config::ventilation_night_stage_OUT = "";
std::string Config::ventilation_night_time_end_OUT = "";
std::string Config::ventilation_night_time_start_OUT = "";
std::string Config::compressor_blockage_duration_OUT = "";
std::string Config::is_surpluspower_heatpump_enabled_OUT = "";
std::string Config::is_surpluspower_grid_export_enabled_OUT = "";
std::string Config::is_surpluspower_immersionheater_enabled_OUT = "";

bool Config::LoadConfig()
{
    logInfo("Loading Config File");
    try
    {
        std::ifstream configFile(filePath);
        if (!configFile.is_open())
        {
            logError("Config file not found.");
            return false;
        }

        std::string line;
        while (std::getline(configFile, line))
        {
            // Entfernen von Leerzeichen und Zeilenumbrüchen
            line = trim(line);

            // Ignoriere Kommentare und leere Zeilen
            if (line.empty() || line[0] == '#' || line[0] == '[')
                continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos)
            {
                logError("Invalid config entry: " + line);
                continue;
            }

            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));

            if (value.empty())
                continue;

            // Werte zuweisen
            if (key == "LxIP")
                LoxoneIP = value;
            else if (key == "LxUser")
                LoxoneUser = value;
            else if (key == "LxPW")
                LoxonePW = value;
            else if (key == "PiceaIP")
                PiceaIP = value;
            else if (key == "PiceaPort")
                PiceaPort = value;
            else if (key == "PiceaJWT")
                PiceaJWT = value;
            else if (key == "autarky")
                autarky = value;
            else if (key == "battery_input_power")
                battery_input_power = value;
            else if (key == "battery_output_power")
                battery_output_power = value;
            else if (key == "battery_power")
                battery_power = value;
            else if (key == "battery_state_of_charge")
                battery_state_of_charge = value;
            else if (key == "co2_avoidance")
                co2_avoidance = value;
            else if (key == "electrolyser_efficiency_electrical")
                electrolyser_efficiency_electrical = value;
            else if (key == "electrolyser_efficiency_total")
                electrolyser_efficiency_total = value;
            else if (key == "electrolyser_heating_power")
                electrolyser_heating_power = value;
            else if (key == "electrolyser_output_power")
                electrolyser_output_power = value;
            else if (key == "extbattery_input_power")
                extbattery_input_power = value;
            else if (key == "extbattery_output_power")
                extbattery_output_power = value;
            else if (key == "extbattery_power")
                extbattery_power = value;
            else if (key == "fuelcell_efficiency_electrical")
                fuelcell_efficiency_electrical = value;
            else if (key == "fuelcell_efficiency_total")
                fuelcell_efficiency_total = value;
            else if (key == "fuelcell_heating_power")
                fuelcell_heating_power = value;
            else if (key == "fuelcell_input_power")
                fuelcell_input_power = value;
            else if (key == "grid_export_power")
                grid_export_power = value;
            else if (key == "grid_import_power")
                grid_import_power = value;
            else if (key == "grid_power")
                grid_power = value;
            else if (key == "heat_contribution_power")
                heat_contribution_power = value;
            else if (key == "hot_water_tempearture")
                hot_water_tempearture = value;
            else if (key == "house_to_picea_air_humidity")
                house_to_picea_air_humidity = value;
            else if (key == "house_to_picea_air_temperature")
                house_to_picea_air_temperature = value;
            else if (key == "household_input_power")
                household_input_power = value;
            else if (key == "household_input_power_from_battery_picea")
                household_input_power_from_battery_picea = value;
            else if (key == "household_input_power_from_extbattery")
                household_input_power_from_extbattery = value;
            else if (key == "household_input_power_from_grid")
                household_input_power_from_grid = value;
            else if (key == "household_input_power_from_hydrogen")
                household_input_power_from_hydrogen = value;
            else if (key == "household_input_power_from_picea")
                household_input_power_from_picea = value;
            else if (key == "hydrogen_input_power_electrical")
                hydrogen_input_power_electrical = value;
            else if (key == "hydrogen_output_power_electrical")
                hydrogen_output_power_electrical = value;
            else if (key == "hydrogen_power_electrical")
                hydrogen_power_electrical = value;
            else if (key == "hydrogen_state_of_charge")
                hydrogen_state_of_charge = value;
            else if (key == "is_efficiencymode")
                is_efficiencymode = value;
            else if (key == "is_equal_charge_from_net")
                is_equal_charge_from_net = value;
            else if (key == "is_equal_charge_ongoing")
                is_equal_charge_ongoing = value;
            else if (key == "is_grid_offline")
                is_grid_offline = value;
            else if (key == "is_heater_alarm")
                is_heater_alarm = value;
            else if (key == "is_heater_error")
                is_heater_error = value;
            else if (key == "is_hydrogen_used_except_reserve")
                is_hydrogen_used_except_reserve = value;
            else if (key == "is_in_ten_percent_grid_feedin_mode")
                is_in_ten_percent_grid_feedin_mode = value;
            else if (key == "is_launchphase")
                is_launchphase = value;
            else if (key == "is_solar_error")
                is_solar_error = value;
            else if (key == "is_ventilation_calibration_now")
                is_ventilation_calibration_now = value;
            else if (key == "is_ventilation_differential_pressure_alarm")
                is_ventilation_differential_pressure_alarm = value;
            else if (key == "is_ventilation_filter_full_alarm")
                is_ventilation_filter_full_alarm = value;
            else if (key == "is_ventilation_filter_full_warning")
                is_ventilation_filter_full_warning = value;
            else if (key == "is_water_error")
                is_water_error = value;
            else if (key == "max_compressor_blockage_duration")
                max_compressor_blockage_duration = value;
            else if (key == "outdoor_to_picea_air_temperature")
                outdoor_to_picea_air_temperature = value;
            else if (key == "picea_to_house_air_temperature")
                picea_to_house_air_temperature = value;
            else if (key == "solar_output_power")
                solar_output_power = value;
            else if (key == "solar_output_power_ac")
                solar_output_power_ac = value;
            else if (key == "solar_output_power_dc_total")
                solar_output_power_dc_total = value;
            else if (key == "solar_output_power_to_battery")
                solar_output_power_to_battery = value;
            else if (key == "solar_output_power_to_extbattery")
                solar_output_power_to_extbattery = value;
            else if (key == "solar_output_power_to_grid")
                solar_output_power_to_grid = value;
            else if (key == "solar_output_power_to_household")
                solar_output_power_to_household = value;
            else if (key == "solar_output_power_to_hydrogen")
                solar_output_power_to_hydrogen = value;
            else if (key == "ventilation_stage_real")
                ventilation_stage_real = value;
            // restlicher Input
            else if (key == "party_mode_enabled_IN")
                party_mode_enabled_IN = value;
            else if (key == "vacation_mode_enabled_IN")
                vacation_mode_enabled_IN = value;
            else if (key == "has_grid_tax_feedin_IN")
                has_grid_tax_feedin_IN = value;
            else if (key == "is_grid_connected_system_IN")
                is_grid_connected_system_IN = value;
            else if (key == "has_no_hot_water_integrated_IN")
                has_no_hot_water_integrated_IN = value;
            else if (key == "has_differential_pressure_gauge_IN")
                has_differential_pressure_gauge_IN = value;
            else if (key == "hydrogen_reserve_IN")
                hydrogen_reserve_IN = value;
            else if (key == "ext_battery_setup_IN")
                ext_battery_setup_IN = value;
            else if (key == "filter_exchange_state_IN")
                filter_exchange_state_IN = value;
            else if (key == "ventilation_stage_user_IN")
                ventilation_stage_user_IN = value;
            else if (key == "ventilation_temperature_target_IN")
                ventilation_temperature_target_IN = value;
            else if (key == "ventilation_night_enabled_IN")
                ventilation_night_enabled_IN = value;
            else if (key == "ventilation_night_stage_IN")
                ventilation_night_stage_IN = value;
            else if (key == "ventilation_night_time_end_IN")
                ventilation_night_time_end_IN = value;
            else if (key == "ventilation_night_time_start_IN")
                ventilation_night_time_start_IN = value;
            else if (key == "compressor_blockage_duration_IN")
                compressor_blockage_duration_IN = value;
            else if (key == "is_surpluspower_heatpump_enabled_IN")
                is_surpluspower_heatpump_enabled_IN = value;
            else if (key == "is_surpluspower_grid_export_enabled_IN")
                is_surpluspower_grid_export_enabled_IN = value;
            else if (key == "is_surpluspower_immersionheater_enabled_IN")
                is_surpluspower_immersionheater_enabled_IN = value;
            else if (key == "picea_generation_IN")
                picea_generation_IN = value;
            else if (key == "picea_count_IN")
                picea_count_IN = value;
            else if (key == "picea_serial_IN")
                picea_serial_IN = value;
            else if (key == "has_solar_dc_IN")
                has_solar_dc_IN = value;
            else if (key == "has_solar_ac_IN")
                has_solar_ac_IN = value;
            else if (key == "has_solar_ac_and_dc_IN")
                has_solar_ac_and_dc_IN = value;
            else if (key == "has_sgready_IN")
                has_sgready_IN = value;
            else if (key == "has_no_heat_contribution_IN")
                has_no_heat_contribution_IN = value;
            else if (key == "has_immersion_heater_IN")
                has_immersion_heater_IN = value;
            else if (key == "has_ventilation_integrated_IN")
                has_ventilation_integrated_IN = value;
            // Output
            else if (key == "party_mode_enabled_OUT")
                party_mode_enabled_OUT = value;
            else if (key == "vacation_mode_enabled_OUT")
                vacation_mode_enabled_OUT = value;
            else if (key == "has_grid_tax_feedin_OUT")
                has_grid_tax_feedin_OUT = value;
            else if (key == "is_grid_connected_system_OUT")
                is_grid_connected_system_OUT = value;
            else if (key == "has_no_hot_water_integrated_OUT")
                has_no_hot_water_integrated_OUT = value;
            else if (key == "has_differential_pressure_gauge_OUT")
                has_differential_pressure_gauge_OUT = value;
            else if (key == "hydrogen_reserve_OUT")
                hydrogen_reserve_OUT = value;
            else if (key == "ext_battery_setup_OUT")
                ext_battery_setup_OUT = value;
            else if (key == "filter_exchange_state_OUT")
                filter_exchange_state_OUT = value;
            else if (key == "ventilation_stage_user_OUT")
                ventilation_stage_user_OUT = value;
            else if (key == "ventilation_temperature_target_OUT")
                ventilation_temperature_target_OUT = value;
            else if (key == "ventilation_night_enabled_OUT")
                ventilation_night_enabled_OUT = value;
            else if (key == "ventilation_night_stage_OUT")
                ventilation_night_stage_OUT = value;
            else if (key == "ventilation_night_time_end_OUT")
                ventilation_night_time_end_OUT = value;
            else if (key == "ventilation_night_time_start_OUT")
                ventilation_night_time_start_OUT = value;
            else if (key == "compressor_blockage_duration_OUT")
                compressor_blockage_duration_OUT = value;
            else if (key == "is_surpluspower_heatpump_enabled_OUT")
                is_surpluspower_heatpump_enabled_OUT = value;
            else if (key == "is_surpluspower_grid_export_enabled_OUT")
                is_surpluspower_grid_export_enabled_OUT = value;
            else if (key == "is_surpluspower_immersionheater_enabled_OUT")
                is_surpluspower_immersionheater_enabled_OUT = value;
        }

        configFile.close();
        logInfo("Config file loaded successfully.");
        return true;
    }
    catch (const std::exception& ex)
    {
        logError("Config file Error: " + std::string(ex.what()));
        return false;
    }
}

std::string Config::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

void Config::logError(const std::string& message)
{
    std::cerr << "[ERROR] " << message << std::endl;
}

void Config::logInfo(const std::string& message)
{
    std::cout << "[INFO] " << message << std::endl;
}
