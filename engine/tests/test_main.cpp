#include <iostream>
#include <Eigen/Dense>
#include <nlohmann/json.hpp>

int main() {
    std::cout << "Midas Test Program" << std::endl;
    std::cout << "==================" << std::endl;

    // Test Eigen
    Eigen::VectorXd vec(3);
    vec << 1.0, 2.0, 3.0;
    std::cout << "✓ Eigen3 loaded successfully" << std::endl;
    std::cout << "  Sample vector: [" << vec.transpose() << "]" << std::endl;

    // Test nlohmann/json
    nlohmann::json j = {
        {"name", "Midas"},
        {"version", "1.0.0"},
        {"status", "initializing"}
    };
    std::cout << "✓ nlohmann/json loaded successfully" << std::endl;
    std::cout << "  Sample JSON: " << j.dump() << std::endl;

    std::cout << "\n✅ All dependencies verified!" << std::endl;
    std::cout << "Ready to build Midas simulation engine." << std::endl;

    return 0;
}
