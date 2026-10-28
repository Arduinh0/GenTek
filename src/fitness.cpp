#include "fitness.hpp"
#include "distance.hpp"
#include <limits>

// ============================================================================
// fitness.cpp — Implementação da função de aptidão
// ============================================================================

double fitness(const Individual& route, const DistMatrix& dist) {
    const double distance = routeDistance(route, dist);

    // Caso degenerado: rota com 0 ou 1 cidade → distância 0
    if (distance == 0.0) {
        return std::numeric_limits<double>::max();
    }

    return 1.0 / distance;
}
