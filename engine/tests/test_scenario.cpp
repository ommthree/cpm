#include "midas/scenario.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace midas;

void test_load_single_scenario() {
    std::cout << "Test: Load Single Scenario..." << std::endl;

    ClimateScenario scenario;
    scenario.load_from_csv("../../data/scenarios/net_zero_2050.csv");

    // Check basic properties
    assert(scenario.name() == "Net Zero 2050");
    assert(scenario.year() > 2020 && scenario.year() <= 2050);

    std::cout << "  ✓ Scenario: " << scenario.name() << " @ " << scenario.year() << std::endl;

    // Check driver access
    double carbon_price_na = scenario.get_driver(0, 0); // CarbonPrice, North America
    std::cout << "  ✓ Carbon price (North America): " << carbon_price_na << "σ" << std::endl;

    // Check regional drivers
    Eigen::VectorXd regional_drivers = scenario.get_regional_drivers(0);
    assert(regional_drivers.size() == 8);
    std::cout << "  ✓ Regional drivers vector: 8×1" << std::endl;

    // Check all drivers
    Eigen::VectorXd all_drivers = scenario.get_all_drivers();
    assert(all_drivers.size() == 56); // 8 drivers × 7 regions
    std::cout << "  ✓ All drivers vector: 56×1" << std::endl;
}

void test_scenario_library() {
    std::cout << "\nTest: Scenario Library..." << std::endl;

    ScenarioLibrary library;
    library.load_all("../../data/scenarios");

    // List scenarios
    auto scenario_names = library.list_scenarios();
    std::cout << "  Available scenarios (" << scenario_names.size() << "):" << std::endl;
    for (const auto& name : scenario_names) {
        auto years = library.list_years(name);
        std::cout << "    - " << name << " (years: ";
        for (size_t i = 0; i < years.size(); ++i) {
            std::cout << years[i];
            if (i < years.size() - 1) std::cout << ", ";
        }
        std::cout << ")" << std::endl;
    }

    // Test scenario retrieval
    if (library.has("Net Zero 2050", 2030)) {
        const auto& scenario = library.get("Net Zero 2050", 2030);
        std::cout << "  ✓ Retrieved: " << scenario.name() << " @ " << scenario.year() << std::endl;
    }
}

void test_driver_values() {
    std::cout << "\nTest: Driver Values..." << std::endl;

    ScenarioLibrary library;
    library.load_all("../../data/scenarios");

    // Compare drivers across scenarios
    std::cout << "  Carbon Price comparison (North America):" << std::endl;

    for (const auto& scenario_name : library.list_scenarios()) {
        auto years = library.list_years(scenario_name);
        if (!years.empty()) {
            int year = years[0]; // Take first available year
            const auto& scenario = library.get(scenario_name, year);
            double carbon_price = scenario.get_driver(0, 0); // CarbonPrice, NA

            std::cout << "    " << scenario_name << " @ " << year
                      << ": " << carbon_price << "σ" << std::endl;
        }
    }
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "Scenario Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    try {
        test_load_single_scenario();
        test_scenario_library();
        test_driver_values();

        std::cout << "\n✅ All scenario tests passed!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
