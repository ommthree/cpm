#include "midas/scenario.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <filesystem>

namespace midas {

// Driver name mappings (matching CSV files)
static const std::vector<std::string> DRIVER_NAMES = {
    "CarbonPrice",
    "CoalPrice",
    "OilPrice",
    "GasPrice",
    "GDP",
    "HeatIndex",
    "FloodRisk",
    "DroughtRisk"
};

// Region name mappings (matching CSV files)
static const std::vector<std::string> REGION_NAMES = {
    "North America",
    "Europe",
    "Asia-Pacific (developed)",
    "Asia-Pacific (emerging)",
    "Latin America",
    "Middle East & Africa",
    "Global"
};

// ClimateScenario methods

ClimateScenario::ClimateScenario(const std::string& csv_path) {
    load_from_csv(csv_path);
}

void ClimateScenario::load_from_csv(const std::string& csv_path) {
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open scenario file: " + csv_path);
    }

    // Initialize driver values to zero
    for (auto& region : driver_values_) {
        region.fill(0.0);
    }

    std::string line;

    // Read header
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty scenario file");
    }

    // Parse header to find column indices
    std::vector<std::string> headers;
    std::stringstream header_ss(line);
    std::string header;
    while (std::getline(header_ss, header, ',')) {
        headers.push_back(header);
    }

    // Find required columns
    auto find_col = [&headers](const std::string& name) -> int {
        auto it = std::find(headers.begin(), headers.end(), name);
        if (it == headers.end()) {
            throw std::runtime_error("Missing required column: " + name);
        }
        return std::distance(headers.begin(), it);
    };

    int col_scenario = find_col("ScenarioName");
    int col_year = find_col("Year");
    int col_driver = find_col("Driver");
    int col_region = find_col("Region");
    int col_value = find_col("StandardizedValue");

    // Read data rows
    int line_num = 1;
    bool name_set = false;
    bool year_set = false;

    while (std::getline(file, line)) {
        ++line_num;

        if (line.empty()) continue;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;

        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() < headers.size()) {
            std::cerr << "Warning: Line " << line_num << " has fewer fields than expected" << std::endl;
            continue;
        }

        try {
            // Extract scenario name and year (should be consistent)
            std::string scenario_name = fields[col_scenario];
            int year = std::stoi(fields[col_year]);

            if (!name_set) {
                name_ = scenario_name;
                name_set = true;
            }

            if (!year_set) {
                year_ = year;
                year_set = true;
            }

            // Parse driver and region
            std::string driver_name = fields[col_driver];
            std::string region_name = fields[col_region];

            int driver_id = parse_driver(driver_name);
            int region_id = parse_region(region_name);

            // Parse standardized value
            double value = std::stod(fields[col_value]);

            // Store driver value
            driver_values_[region_id][driver_id] = value;

        } catch (const std::exception& e) {
            std::cerr << "Error parsing line " << line_num << ": " << e.what() << std::endl;
            // Continue processing other lines
        }
    }

    file.close();

    if (!name_set || !year_set) {
        throw std::runtime_error("Failed to determine scenario name or year from " + csv_path);
    }

    std::cout << "Loaded scenario: " << name_ << " @ " << year_ << " from " << csv_path << std::endl;
}

double ClimateScenario::get_driver(int driver_id, int region_id) const {
    if (driver_id < 0 || driver_id >= 8) {
        throw std::out_of_range("Driver ID must be in [0, 7], got " + std::to_string(driver_id));
    }
    if (region_id < 0 || region_id >= 7) {
        throw std::out_of_range("Region ID must be in [0, 6], got " + std::to_string(region_id));
    }

    return driver_values_[region_id][driver_id];
}

Eigen::VectorXd ClimateScenario::get_regional_drivers(int region_id) const {
    if (region_id < 0 || region_id >= 7) {
        throw std::out_of_range("Region ID must be in [0, 6], got " + std::to_string(region_id));
    }

    Eigen::VectorXd drivers(8);
    for (int i = 0; i < 8; ++i) {
        drivers(i) = driver_values_[region_id][i];
    }

    return drivers;
}

Eigen::VectorXd ClimateScenario::get_all_drivers() const {
    Eigen::VectorXd drivers(56); // 8 drivers × 7 regions

    int idx = 0;
    for (int r = 0; r < 7; ++r) {
        for (int d = 0; d < 8; ++d) {
            drivers(idx++) = driver_values_[r][d];
        }
    }

    return drivers;
}

int ClimateScenario::parse_driver(const std::string& driver_name) const {
    auto it = std::find(DRIVER_NAMES.begin(), DRIVER_NAMES.end(), driver_name);

    if (it != DRIVER_NAMES.end()) {
        return std::distance(DRIVER_NAMES.begin(), it);
    }

    throw std::runtime_error("Unknown driver: " + driver_name);
}

int ClimateScenario::parse_region(const std::string& region_name) const {
    auto it = std::find(REGION_NAMES.begin(), REGION_NAMES.end(), region_name);

    if (it != REGION_NAMES.end()) {
        return std::distance(REGION_NAMES.begin(), it);
    }

    throw std::runtime_error("Unknown region: " + region_name);
}

// ScenarioLibrary methods

void ScenarioLibrary::load_all(const std::string& scenarios_dir) {
    namespace fs = std::filesystem;

    if (!fs::exists(scenarios_dir)) {
        throw std::runtime_error("Scenarios directory not found: " + scenarios_dir);
    }

    // Iterate through CSV files in directory
    for (const auto& entry : fs::directory_iterator(scenarios_dir)) {
        if (entry.path().extension() == ".csv") {
            try {
                load_scenario(entry.path().string());
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to load " << entry.path() << ": " << e.what() << std::endl;
            }
        }
    }

    std::cout << "Loaded " << scenarios_.size() << " scenario(s) from " << scenarios_dir << std::endl;
}

void ScenarioLibrary::load_scenario(const std::string& csv_path) {
    ClimateScenario scenario(csv_path);

    // Store in nested map: name -> year -> scenario
    scenarios_[scenario.name()][scenario.year()] = scenario;
}

const ClimateScenario& ScenarioLibrary::get(const std::string& name, int year) const {
    auto name_it = scenarios_.find(name);
    if (name_it == scenarios_.end()) {
        throw std::runtime_error("Scenario not found: " + name);
    }

    auto year_it = name_it->second.find(year);
    if (year_it == name_it->second.end()) {
        throw std::runtime_error("Year " + std::to_string(year) + " not found for scenario: " + name);
    }

    return year_it->second;
}

bool ScenarioLibrary::has(const std::string& name, int year) const {
    auto name_it = scenarios_.find(name);
    if (name_it == scenarios_.end()) {
        return false;
    }

    return name_it->second.find(year) != name_it->second.end();
}

std::vector<std::string> ScenarioLibrary::list_scenarios() const {
    std::vector<std::string> names;

    for (const auto& [name, years] : scenarios_) {
        names.push_back(name);
    }

    return names;
}

std::vector<int> ScenarioLibrary::list_years(const std::string& name) const {
    std::vector<int> years;

    auto it = scenarios_.find(name);
    if (it != scenarios_.end()) {
        for (const auto& [year, scenario] : it->second) {
            years.push_back(year);
        }
    }

    return years;
}

std::string ScenarioLibrary::make_key(const std::string& name, int year) const {
    return name + "_" + std::to_string(year);
}

} // namespace midas
