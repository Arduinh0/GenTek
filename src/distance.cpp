#include "distance.hpp"
#include <cmath>
#include <stdexcept>

// ============================================================================
// distance.cpp — Implementação do pré-cálculo de distâncias e avaliação de rotas
// ============================================================================

double euclideanDistance(const Point& a, const Point& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

DistMatrix computeDistanceMatrix(const std::vector<Point>& cities) {
    const int n = static_cast<int>(cities.size());

    // Inicializa matriz NxN com zeros
    DistMatrix matrix(n, std::vector<double>(n, 0.0));

    // Calcula apenas o triângulo superior e espelha (simetria)
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const double d = euclideanDistance(cities[i], cities[j]);
            matrix[i][j] = d;
            matrix[j][i] = d;
        }
    }

    return matrix;
}

double routeDistance(const Individual& route, const DistMatrix& dist) {
    if (route.empty()) {
        return 0.0;
    }

    const int n = static_cast<int>(route.size());
    double totalDistance = 0.0;

    // Soma as distâncias consecutivas: route[0]→route[1]→...→route[N-1]
    for (int i = 0; i < n - 1; ++i) {
        totalDistance += dist[route[i]][route[i + 1]];
    }

    // Fecha o ciclo: route[N-1] → route[0]
    totalDistance += dist[route[n - 1]][route[0]];

    return totalDistance;
}
