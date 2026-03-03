#include "midas/portfolio.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace midas;

void test_load_portfolio() {
    std::cout << "Test: Load Portfolio from CSV..." << std::endl;

    Portfolio portfolio;
    portfolio.load_from_csv("../../data/portfolios/sample_portfolio_500.csv");

    // Check size
    assert(portfolio.size() == 500);
    std::cout << "  ✓ Loaded 500 obligors" << std::endl;

    // Check total EAD
    double total_ead = portfolio.total_ead();
    assert(total_ead > 0.0);
    std::cout << "  ✓ Total EAD: $" << (total_ead / 1e9) << "B" << std::endl;

    // Check first obligor
    const auto& first = portfolio.get_obligor(0);
    assert(!first.id.empty());
    assert(first.pd > 0.0 && first.pd < 1.0);
    assert(first.lgd > 0.0 && first.lgd < 1.0);
    assert(first.ead > 0.0);
    assert(first.beta.size() == 105);

    std::cout << "  ✓ First obligor: " << first.id << " (" << first.name << ")" << std::endl;
    std::cout << "    Sector: " << first.sector_id << ", Region: " << first.region_id << std::endl;
    std::cout << "    PD: " << first.pd << ", LGD: " << first.lgd << ", EAD: $" << (first.ead / 1e6) << "M" << std::endl;
    std::cout << "    Threshold: " << first.threshold << std::endl;
    std::cout << "    Beta sum: " << first.beta.sum() << std::endl;
}

void test_validate_portfolio() {
    std::cout << "\nTest: Validate Portfolio..." << std::endl;

    Portfolio portfolio("../../data/portfolios/sample_portfolio_500.csv");

    try {
        portfolio.validate();
        std::cout << "  ✓ Portfolio validation passed" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  ✗ Validation failed: " << e.what() << std::endl;
        throw;
    }
}

void test_sector_region_analysis() {
    std::cout << "\nTest: Sector/Region Analysis..." << std::endl;

    Portfolio portfolio("../../data/portfolios/sample_portfolio_500.csv");

    // Sector EAD
    auto sector_ead = portfolio.sector_ead();
    std::cout << "  Sector EAD breakdown:" << std::endl;
    for (const auto& [sector_id, ead] : sector_ead) {
        std::cout << "    Sector " << sector_id << ": $" << (ead / 1e9) << "B" << std::endl;
    }

    // Region EAD
    auto region_ead = portfolio.region_ead();
    std::cout << "  Region EAD breakdown:" << std::endl;
    for (const auto& [region_id, ead] : region_ead) {
        std::cout << "    Region " << region_id << ": $" << (ead / 1e9) << "B" << std::endl;
    }

    // Concentration matrix
    auto conc_matrix = portfolio.concentration_matrix();
    std::cout << "  ✓ Concentration matrix: " << conc_matrix.rows() << "x" << conc_matrix.cols() << std::endl;

    // Check that matrix sum equals total EAD
    double matrix_sum = conc_matrix.sum();
    double total_ead = portfolio.total_ead();
    assert(std::abs(matrix_sum - total_ead) < 1.0);
    std::cout << "  ✓ Matrix sum matches total EAD" << std::endl;
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "Portfolio Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    try {
        test_load_portfolio();
        test_validate_portfolio();
        test_sector_region_analysis();

        std::cout << "\n✅ All portfolio tests passed!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
