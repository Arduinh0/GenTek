#include "generators.hpp"
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>

// ============================================================================
// generators.cpp — Implementação dos geradores de cenários
// ============================================================================

// Constante local — evita incluir <numbers> para manter compatibilidade C++17
static constexpr double PI = 3.14159265358979323846;

std::vector<Point> generateRandomPoints(const int n, const unsigned seed) {
    if (n <= 0) {
        throw std::invalid_argument(
            "generateRandomPoints: n deve ser > 0 (recebido: " +
            std::to_string(n) + ")");
    }

    // Se seed == 0, usa random_device para seed não-determinística
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    std::uniform_real_distribution<double> coordDist(0.0, 100.0);

    std::vector<Point> points;
    points.reserve(n);

    for (int i = 0; i < n; ++i) {
        points.push_back({coordDist(rng), coordDist(rng)});
    }

    return points;
}

std::vector<Point> generateCircularPoints(const int n, const double radius,
                                          const Point center) {
    if (n <= 0) {
        throw std::invalid_argument(
            "generateCircularPoints: n deve ser > 0 (recebido: " +
            std::to_string(n) + ")");
    }
    if (radius <= 0.0) {
        throw std::invalid_argument(
            "generateCircularPoints: radius deve ser > 0 (recebido: " +
            std::to_string(radius) + ")");
    }

    std::vector<Point> points;
    points.reserve(n);

    const double angleStep = 2.0 * PI / n;

    for (int i = 0; i < n; ++i) {
        const double theta = angleStep * i;
        points.push_back({
            center.x + radius * std::cos(theta),
            center.y + radius * std::sin(theta)
        });
    }

    return points;
}
