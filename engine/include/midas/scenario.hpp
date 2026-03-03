#pragma once

#include <string>
#include <vector>
#include <map>
#include <array>
#include <Eigen/Dense>

namespace midas {

/**
 * @brief Climate scenario with standardized driver values
 */
class ClimateScenario {
public:
    /**
     * @brief Default constructor
     */
    ClimateScenario() = default;

    /**
     * @brief Construct and load scenario from CSV file
     * @param csv_path Path to scenario CSV file
     */
    explicit ClimateScenario(const std::string& csv_path);

    /**
     * @brief Load scenario from CSV file
     * @param csv_path Path to scenario CSV file
     *
     * Expected CSV format:
     * ScenarioName,Year,Driver,Region,Value,StandardizedValue,Notes,Baseline,Sigma
     */
    void load_from_csv(const std::string& csv_path);

    // Accessors

    /**
     * @brief Get scenario name
     */
    const std::string& name() const { return name_; }

    /**
     * @brief Get scenario year
     */
    int year() const { return year_; }

    /**
     * @brief Get standardized driver value for specific driver and region
     * @param driver_id Driver index [0-7]
     * @param region_id Region index [0-6]
     * @return Standardized φ value
     */
    double get_driver(int driver_id, int region_id) const;

    /**
     * @brief Get all driver values for a specific region
     * @param region_id Region index [0-6]
     * @return 8×1 vector of standardized φ values
     */
    Eigen::VectorXd get_regional_drivers(int region_id) const;

    /**
     * @brief Get all driver values as flattened vector
     * @return 56×1 vector (8 drivers × 7 regions)
     */
    Eigen::VectorXd get_all_drivers() const;

private:
    std::string name_;                         // e.g., "Net Zero 2050"
    int year_;                                 // e.g., 2030

    // Driver values: driver_values_[region_id][driver_id]
    std::array<std::array<double, 8>, 7> driver_values_;

    /**
     * @brief Parse driver name to driver_id
     */
    int parse_driver(const std::string& driver_name) const;

    /**
     * @brief Parse region name to region_id
     */
    int parse_region(const std::string& region_name) const;
};

/**
 * @brief Library to manage multiple climate scenarios
 */
class ScenarioLibrary {
public:
    /**
     * @brief Default constructor
     */
    ScenarioLibrary() = default;

    /**
     * @brief Load all scenario CSV files from a directory
     * @param scenarios_dir Path to directory containing scenario CSV files
     */
    void load_all(const std::string& scenarios_dir);

    /**
     * @brief Load a single scenario file
     * @param csv_path Path to scenario CSV file
     */
    void load_scenario(const std::string& csv_path);

    /**
     * @brief Get scenario by name and year
     * @param name Scenario name (e.g., "Net Zero 2050")
     * @param year Scenario year (e.g., 2030)
     * @return Reference to ClimateScenario
     * @throws std::runtime_error if scenario not found
     */
    const ClimateScenario& get(const std::string& name, int year) const;

    /**
     * @brief Check if scenario exists
     * @param name Scenario name
     * @param year Scenario year
     * @return true if scenario exists
     */
    bool has(const std::string& name, int year) const;

    /**
     * @brief Get list of all available scenario names
     * @return Vector of unique scenario names
     */
    std::vector<std::string> list_scenarios() const;

    /**
     * @brief Get list of years for a specific scenario
     * @param name Scenario name
     * @return Vector of available years
     */
    std::vector<int> list_years(const std::string& name) const;

private:
    // Map: scenario_name -> (year -> ClimateScenario)
    std::map<std::string, std::map<int, ClimateScenario>> scenarios_;

    /**
     * @brief Create unique key for scenario lookup
     */
    std::string make_key(const std::string& name, int year) const;
};

} // namespace midas
