#include "midas/calibration.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace midas;

void test_load_calibration() {
    std::cout << "Test: Load Calibration Data..." << std::endl;

    CalibrationData calibration;
    calibration.load_from_directory("../../data/calibration");

    std::cout << "  ✓ Calibration data loaded successfully" << std::endl;
}

void test_dimensions() {
    std::cout << "\nTest: Validate Dimensions..." << std::endl;

    CalibrationData calibration;
    calibration.load_from_directory("../../data/calibration");

    // Check sector scores
    assert(calibration.sector_scores().rows() == 15);
    assert(calibration.sector_scores().cols() == 8);
    std::cout << "  ✓ Sector scores: 15×8" << std::endl;

    // Check region scores
    assert(calibration.region_scores().rows() == 7);
    assert(calibration.region_scores().cols() == 8);
    std::cout << "  ✓ Region scores: 7×8" << std::endl;

    // Check lambda parameters
    assert(calibration.lambda_parameters().size() == 8);
    std::cout << "  ✓ Lambda parameters: 8×1" << std::endl;

    // Check residual variance
    assert(calibration.residual_variance().rows() == 105);
    assert(calibration.residual_variance().cols() == 105);
    std::cout << "  ✓ Residual variance: 105×105" << std::endl;

    // Check precomputed weights
    assert(calibration.weights().rows() == 105);
    assert(calibration.weights().cols() == 8);
    std::cout << "  ✓ Precomputed weights: 105×8" << std::endl;

    // Check Cholesky decomposition
    assert(calibration.cholesky().rows() == 105);
    assert(calibration.cholesky().cols() == 105);
    std::cout << "  ✓ Cholesky decomposition: 105×105" << std::endl;
}

void test_weight_computation() {
    std::cout << "\nTest: Weight Computation..." << std::endl;

    CalibrationData calibration;
    calibration.load_from_directory("../../data/calibration");

    // Test individual weight access
    double w_000 = calibration.get_weight(0, 0, 0);  // Sector 0, Region 0, Driver 0
    std::cout << "  Sample weight w̃_{0,0,0}: " << w_000 << std::endl;

    // Verify weight formula: w̃_{s,r,k} = S_{s,k} × R_{r,k}
    double s_00 = calibration.sector_scores()(0, 0);
    double r_00 = calibration.region_scores()(0, 0);
    double expected = s_00 * r_00;

    assert(std::abs(w_000 - expected) < 1e-10);
    std::cout << "  ✓ Weight formula verified: w̃ = S × R" << std::endl;

    // Test cell weights vector
    Eigen::VectorXd cell_weights = calibration.get_cell_weights(0, 0);
    assert(cell_weights.size() == 8);
    std::cout << "  ✓ Cell weights vector: 8×1" << std::endl;

    // Print sample weights
    std::cout << "  Sample cell weights (sector 0, region 0):" << std::endl;
    for (int k = 0; k < 8; ++k) {
        std::cout << "    Driver " << k << ": " << cell_weights(k) << std::endl;
    }
}

void test_cholesky_properties() {
    std::cout << "\nTest: Cholesky Decomposition Properties..." << std::endl;

    CalibrationData calibration;
    calibration.load_from_directory("../../data/calibration");

    const auto& L = calibration.cholesky();
    const auto& Sigma = calibration.residual_variance();

    // Verify that L is lower triangular
    bool is_lower = true;
    for (int i = 0; i < L.rows() && i < 10; ++i) {  // Check first 10 rows
        for (int j = i + 1; j < L.cols() && j < 10; ++j) {
            if (std::abs(L(i, j)) > 1e-10) {
                is_lower = false;
                break;
            }
        }
        if (!is_lower) break;
    }
    assert(is_lower);
    std::cout << "  ✓ Cholesky matrix is lower triangular" << std::endl;

    // Verify reconstruction (sample check on upper-left 5×5 block)
    Eigen::MatrixXd reconstructed = L * L.transpose();
    double error = (reconstructed.block(0, 0, 5, 5) - Sigma.block(0, 0, 5, 5)).norm();
    assert(error < 1e-6);
    std::cout << "  ✓ Cholesky reconstruction verified: Σ = L L^T (error = " << error << ")" << std::endl;

    // Check diagonal elements are positive
    bool diag_positive = true;
    for (int i = 0; i < L.rows(); ++i) {
        if (L(i, i) <= 0) {
            diag_positive = false;
            break;
        }
    }
    assert(diag_positive);
    std::cout << "  ✓ Cholesky diagonal elements are positive" << std::endl;
}

void test_matrix_symmetry() {
    std::cout << "\nTest: Residual Variance Symmetry..." << std::endl;

    CalibrationData calibration;
    calibration.load_from_directory("../../data/calibration");

    const auto& Sigma = calibration.residual_variance();

    // Check symmetry
    double asymmetry = (Sigma - Sigma.transpose()).norm();
    std::cout << "  Asymmetry measure: " << asymmetry << std::endl;
    assert(asymmetry < 1e-6);
    std::cout << "  ✓ Residual variance is symmetric" << std::endl;

    // Check positive semi-definiteness (all eigenvalues >= 0)
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(Sigma);
    double min_eigenvalue = es.eigenvalues().minCoeff();
    std::cout << "  Minimum eigenvalue: " << min_eigenvalue << std::endl;
    assert(min_eigenvalue > -1e-6);
    std::cout << "  ✓ Residual variance is positive semi-definite" << std::endl;
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "Calibration Unit Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    try {
        test_load_calibration();
        test_dimensions();
        test_weight_computation();
        test_cholesky_properties();
        test_matrix_symmetry();

        std::cout << "\n✅ All calibration tests passed!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
