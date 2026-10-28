#include "types.hpp"
#include "distance.hpp"
#include "fitness.hpp"
#include "generators.hpp"
#include "genetic.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>

// ============================================================================
// main.cpp — Driver de Testes: Fase 1 (validação) + Fase 2 (AG completo)
// ============================================================================

static constexpr double PI = 3.14159265358979323846;

/// Imprime as coordenadas de todas as cidades.
void printCities(const std::vector<Point>& cities) {
    for (int i = 0; i < static_cast<int>(cities.size()); ++i) {
        std::cout << "  Cidade " << i << ": ("
                  << std::fixed << std::setprecision(2)
                  << cities[i].x << ", " << cities[i].y << ")\n";
    }
}

/// Imprime uma rota formatada.
void printRoute(const Individual& route) {
    std::cout << "  [";
    for (int i = 0; i < static_cast<int>(route.size()); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << route[i];
    }
    std::cout << " -> " << route[0] << "]\n";
}

/// Separador visual entre seções.
void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n"
              << title << "\n"
              << std::string(60, '=') << "\n\n";
}

int main() {
    // ========================================================================
    // FASE 1 — Validação rápida (cenário circular benchmark)
    // ========================================================================
    printSeparator("FASE 1: Validacao Rapida (Circular, 8 cidades, raio 40)");

    const int nCircular = 8;
    const double radius = 40.0;
    const auto circularCities = generateCircularPoints(nCircular, radius);
    const auto circularDist = computeDistanceMatrix(circularCities);

    Individual seqRoute(nCircular);
    std::iota(seqRoute.begin(), seqRoute.end(), 0);

    const double circularRouteDist = routeDistance(seqRoute, circularDist);
    const double theoreticalPerimeter =
        2.0 * nCircular * radius * std::sin(PI / nCircular);
    const double diff = std::abs(circularRouteDist - theoreticalPerimeter);

    std::cout << "  Dist. calculada: " << std::fixed << std::setprecision(4)
              << circularRouteDist << "\n"
              << "  Dist. teorica:   " << theoreticalPerimeter << "\n"
              << "  Verificacao:     " << (diff < 1e-6 ? "PASSOU" : "FALHOU")
              << " (diff = " << std::scientific << diff << ")\n";

    // ========================================================================
    // FASE 2 — Algoritmo Genético em cenário aleatório (20 cidades)
    // ========================================================================
    printSeparator("FASE 2: Algoritmo Genetico (20 cidades aleatorias)");

    const int numCities = 20;
    const unsigned scenarioSeed = 42;

    const auto cities = generateRandomPoints(numCities, scenarioSeed);
    const auto dist = computeDistanceMatrix(cities);

    std::cout << "Cidades geradas (seed = " << scenarioSeed << "):\n";
    printCities(cities);

    // Configuração do AG
    GAConfig config;
    config.populationSize  = 200;
    config.tournamentSize  = 3;
    config.crossoverRate   = 0.85;
    config.mutationRate    = 0.03;
    config.eliteCount      = 2;
    config.maxGenerations  = 1500;
    config.stagnationLimit = 250;
    config.seed            = 123;  // Seed fixa para reprodutibilidade do AG

    std::cout << "\nParametros do AG:\n"
              << "  Populacao:     " << config.populationSize << "\n"
              << "  Torneio (k):   " << config.tournamentSize << "\n"
              << "  Cruzamento:    " << config.crossoverRate * 100 << "%\n"
              << "  Mutacao:       " << config.mutationRate * 100 << "%\n"
              << "  Elitismo:      " << config.eliteCount << " individuos\n"
              << "  Max Geracoes:  " << config.maxGenerations << "\n"
              << "  Estagnacao:    " << config.stagnationLimit << " geracoes\n"
              << "  Seed AG:       " << config.seed << "\n\n";

    std::cout << "Evolucao:\n";

    // Executar o AG
    const GAResult result = runGA(cities, dist, config);

    // Resultado final
    printSeparator("RESULTADO FINAL");

    std::cout << "  Razao de paragem:    " << result.stopReason << "\n"
              << "  Geracoes executadas: " << result.generationsRun << "\n\n"
              << "  Melhor dist. inicial:  " << std::fixed << std::setprecision(4)
              << result.initialBestDistance << "\n"
              << "  Melhor dist. final:    " << result.bestDistance << "\n"
              << "  Melhoria:              "
              << std::setprecision(2)
              << ((result.initialBestDistance - result.bestDistance)
                  / result.initialBestDistance * 100.0)
              << "%\n\n"
              << "  Fitness final:         " << std::scientific
              << std::setprecision(6) << result.bestFitness << "\n\n"
              << "  Melhor rota encontrada:\n";
    printRoute(result.bestRoute);

    // ========================================================================
    // FASE 2 (BONUS) — AG no cenário circular (validação de convergência)
    // ========================================================================
    printSeparator("BONUS: AG no Cenario Circular (8 cidades, raio 40)");

    GAConfig circConfig;
    circConfig.populationSize  = 100;
    circConfig.tournamentSize  = 3;
    circConfig.crossoverRate   = 0.85;
    circConfig.mutationRate    = 0.05;
    circConfig.eliteCount      = 2;
    circConfig.maxGenerations  = 500;
    circConfig.stagnationLimit = 100;
    circConfig.seed            = 456;

    std::cout << "Evolucao:\n";
    const GAResult circResult = runGA(circularCities, circularDist, circConfig);

    std::cout << "\n  Dist. otima teorica:   " << std::fixed << std::setprecision(4)
              << theoreticalPerimeter << "\n"
              << "  Dist. encontrada pelo AG: " << circResult.bestDistance << "\n"
              << "  Diferenca:             "
              << std::abs(circResult.bestDistance - theoreticalPerimeter) << "\n"
              << "  Convergiu para o otimo: "
              << (std::abs(circResult.bestDistance - theoreticalPerimeter) < 1e-6
                  ? "SIM" : "NAO") << "\n";

    // ========================================================================
    std::cout << "\n" << std::string(60, '=') << "\n"
              << "Fases 1 e 2 concluidas com sucesso!\n"
              << std::string(60, '=') << "\n";

    return 0;
}
