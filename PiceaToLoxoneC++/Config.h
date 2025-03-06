#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config
{
private:
    static const std::string filePath;

public:
    static std::string LoxoneIP;
    static std::string LoxoneUser;
    static std::string LoxonePW;
    static std::string PiceaIP;
    static std::string PiceaPort;
    static std::string PiceaJWT;

    static bool LoadConfig();

    // Config for VI's
    static std::string autarky;
    static std::string battery_input_power;
    static std::string battery_output_power;
    static std::string battery_power;
    static std::string battery_state_of_charge;
    static std::string co2_avoidance;
    static std::string electrolyser_efficiency_electrical;
    static std::string electrolyser_efficiency_total;
    static std::string electrolyser_heating_power;
    static std::string electrolyser_output_power;
    static std::string extbattery_input_power;
    static std::string extbattery_output_power;
    static std::string extbattery_power;
    static std::string fuelcell_efficiency_electrical;
    static std::string fuelcell_efficiency_total;
    static std::string fuelcell_heating_power;
    static std::string fuelcell_input_power;
    static std::string grid_export_power;
    static std::string grid_import_power;
    static std::string grid_power;
    static std::string heat_contribution_power;
    static std::string hot_water_tempearture;
    static std::string house_to_picea_air_humidity;
    static std::string house_to_picea_air_temperature;
    static std::string household_input_power;
    static std::string household_input_power_from_battery_picea;
    static std::string household_input_power_from_extbattery;
    static std::string household_input_power_from_grid;
    static std::string household_input_power_from_hydrogen;
    static std::string household_input_power_from_picea;
    static std::string hydrogen_input_power_electrical;
    static std::string hydrogen_output_power_electrical;
    static std::string hydrogen_power_electrical;
    static std::string hydrogen_state_of_charge;
    static std::string is_efficiencymode;
    static std::string is_equal_charge_from_net;
    static std::string is_equal_charge_ongoing;
    static std::string is_grid_offline;
    static std::string is_heater_alarm;
    static std::string is_heater_error;
    static std::string is_hydrogen_used_except_reserve;
    static std::string is_in_ten_percent_grid_feedin_mode;
    static std::string is_launchphase;
    static std::string is_solar_error;
    static std::string is_ventilation_calibration_now;
    static std::string is_ventilation_differential_pressure_alarm;
    static std::string is_ventilation_filter_full_alarm;
    static std::string is_ventilation_filter_full_warning;
    static std::string is_water_error;
    static std::string max_compressor_blockage_duration;
    static std::string outdoor_to_picea_air_temperature;
    static std::string picea_to_house_air_temperature;
    static std::string solar_output_power;
    static std::string solar_output_power_ac;
    static std::string solar_output_power_dc_total;
    static std::string solar_output_power_to_battery;
    static std::string solar_output_power_to_extbattery;
    static std::string solar_output_power_to_grid;
    static std::string solar_output_power_to_household;
    static std::string solar_output_power_to_hydrogen;
    static std::string ventilation_stage_real;

    static std::string party_mode_enabled_IN;
    static std::string vacation_mode_enabled_IN;
    static std::string has_grid_tax_feedin_IN;
    static std::string is_grid_connected_system_IN;
    static std::string has_no_hot_water_integrated_IN;
    static std::string has_differential_pressure_gauge_IN;
    static std::string hydrogen_reserve_IN;
    static std::string ext_battery_setup_IN;
    static std::string filter_exchange_state_IN;
    static std::string ventilation_stage_user_IN;
    static std::string ventilation_temperature_target_IN;
    static std::string ventilation_night_enabled_IN;
    static std::string ventilation_night_stage_IN;
    static std::string ventilation_night_time_end_IN;
    static std::string ventilation_night_time_start_IN;
    static std::string compressor_blockage_duration_IN;
    static std::string is_surpluspower_heatpump_enabled_IN;
    static std::string is_surpluspower_grid_export_enabled_IN;
    static std::string is_surpluspower_immersionheater_enabled_IN;
    static std::string picea_generation_IN;
    static std::string picea_count_IN;
    static std::string picea_serial_IN;
    static std::string has_solar_dc_IN;
    static std::string has_solar_ac_IN;
    static std::string has_solar_ac_and_dc_IN;
    static std::string has_sgready_IN;
    static std::string has_no_heat_contribution_IN;
    static std::string has_immersion_heater_IN;
    static std::string has_ventilation_integrated_IN;

    static std::string party_mode_enabled_OUT;
    static std::string vacation_mode_enabled_OUT;
    static std::string has_grid_tax_feedin_OUT;
    static std::string is_grid_connected_system_OUT;
    static std::string has_no_hot_water_integrated_OUT;
    static std::string has_differential_pressure_gauge_OUT;
    static std::string hydrogen_reserve_OUT;
    static std::string ext_battery_setup_OUT;
    static std::string filter_exchange_state_OUT;
    static std::string ventilation_stage_user_OUT;
    static std::string ventilation_temperature_target_OUT;
    static std::string ventilation_night_enabled_OUT;
    static std::string ventilation_night_stage_OUT;
    static std::string ventilation_night_time_end_OUT;
    static std::string ventilation_night_time_start_OUT;
    static std::string compressor_blockage_duration_OUT;
    static std::string is_surpluspower_heatpump_enabled_OUT;
    static std::string is_surpluspower_grid_export_enabled_OUT;
    static std::string is_surpluspower_immersionheater_enabled_OUT;

private:
    static std::string trim(const std::string& str);
    static void logError(const std::string& message);
    static void logInfo(const std::string& message);
};

#endif // CONFIG_H
