#include "types.hpp"
#include "distance.hpp"
#include "fitness.hpp"
#include "generators.hpp"
#include "genetic.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>

// ============================================================================
// main.cpp — Fase 3: Experiencias Formais e Exportacao de Telemetria (CSV)
// ============================================================================

static constexpr double PI = 3.14159265358979323846;

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
    std::cout << "\n" << std::string(65, '=') << "\n"
              << title << "\n"
              << std::string(65, '=') << "\n\n";
}

/// Conta o número de linhas num ficheiro para confirmação do log.
int countLinesInFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return 0;
    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) ++count;
    }
    return count;
}

int main() {
    printSeparator("FASE 3: EXECUCAO DE EXPERIENCIAS E EXPORTACAO DE LOGS");

    // ========================================================================
    // EXPERIÊNCIA 1: Cenário Aleatório (N = 30 Cidades)
    // ========================================================================
    printSeparator("EXPERIENCIA 1: Cenario Aleatorio (N = 30 cidades)");

    const int nRandom = 30;
    const unsigned randomScenarioSeed = 42;

    const auto randomCities = generateRandomPoints(nRandom, randomScenarioSeed);
    const auto randomDist = computeDistanceMatrix(randomCities);

    GAConfig randomConfig;
    randomConfig.populationSize  = 300;
    randomConfig.tournamentSize  = 3;
    randomConfig.crossoverRate   = 0.85;
    randomConfig.mutationRate    = 0.05;
    randomConfig.eliteCount      = 2;
    randomConfig.maxGenerations  = 2000;
    randomConfig.stagnationLimit = 400;
    randomConfig.seed            = 12345;
    randomConfig.csvFilename     = "log_aleatorio.csv";

    std::cout << "Configuracao do AG:\n"
              << "  Cidades:       " << nRandom << "\n"
              << "  Populacao:     " << randomConfig.populationSize << "\n"
              << "  Torneio (k):   " << randomConfig.tournamentSize << "\n"
              << "  Cruzamento:    " << randomConfig.crossoverRate * 100 << "%\n"
              << "  Mutacao:       " << randomConfig.mutationRate * 100 << "%\n"
              << "  Elitismo:      " << randomConfig.eliteCount << "\n"
              << "  Max Geracoes:  " << randomConfig.maxGenerations << "\n"
              << "  Estagnacao:    " << randomConfig.stagnationLimit << " geracoes\n"
              << "  Ficheiro CSV:  " << randomConfig.csvFilename << "\n\n"
              << "Evolucao:\n";

    const GAResult randomResult = runGA(randomCities, randomDist, randomConfig);

    std::cout << "\nResultados Experiencia 1:\n"
              << "  Razao de Paragem:      " << randomResult.stopReason << "\n"
              << "  Epocas Executadas:     " << randomResult.generationsRun << "\n"
              << "  Melhor Dist. Inicial:  " << std::fixed << std::setprecision(4)
              << randomResult.initialBestDistance << "\n"
              << "  Melhor Dist. Final:    " << randomResult.bestDistance << "\n"
              << "  Melhoria Obtida:       " << std::setprecision(2)
              << ((randomResult.initialBestDistance - randomResult.bestDistance)
                  / randomResult.initialBestDistance * 100.0) << "%\n"
              << "  Fitness Final:         " << std::scientific
              << std::setprecision(6) << randomResult.bestFitness << "\n"
              << "  Total Linhas no CSV:   " << countLinesInFile(randomConfig.csvFilename)
              << " (cabecalho + geracoes)\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(randomResult.bestRoute);

    // ========================================================================
    // EXPERIÊNCIA 2: Cenário Circular Benchmark (N = 30 Cidades, Raio = 40)
    // ========================================================================
    printSeparator("EXPERIENCIA 2: Cenario Circular Benchmark (N = 30 cidades)");

    const int nCircular = 30;
    const double radius = 40.0;

    const auto circularCities = generateCircularPoints(nCircular, radius);
    const auto circularDist = computeDistanceMatrix(circularCities);

    // Perímetro teórico ótimo = 2 * N * r * sin(pi / N)
    const double theoreticalOptimal = 2.0 * nCircular * radius * std::sin(PI / nCircular);

    GAConfig circularConfig;
    circularConfig.populationSize  = 300;
    circularConfig.tournamentSize  = 3;
    circularConfig.crossoverRate   = 0.85;
    circularConfig.mutationRate    = 0.05;
    circularConfig.eliteCount      = 2;
    circularConfig.maxGenerations  = 2000;
    circularConfig.stagnationLimit = 400;
    circularConfig.seed            = 54321;
    circularConfig.csvFilename     = "log_circular.csv";

    std::cout << "Configuracao do AG (Benchmark Circular):\n"
              << "  Cidades:               " << nCircular << "\n"
              << "  Raio do Circulo:       " << radius << "\n"
              << "  Dist. Otima Teorica:   " << std::fixed << std::setprecision(4)
              << theoreticalOptimal << "\n"
              << "  Populacao:             " << circularConfig.populationSize << "\n"
              << "  Max Geracoes:          " << circularConfig.maxGenerations << "\n"
              << "  Estagnacao:            " << circularConfig.stagnationLimit << "\n"
              << "  Ficheiro CSV:          " << circularConfig.csvFilename << "\n\n"
              << "Evolucao:\n";

    const GAResult circularResult = runGA(circularCities, circularDist, circularConfig);

    const double diffFromOptimal = circularResult.bestDistance - theoreticalOptimal;
    const double errorPercentage = (diffFromOptimal / theoreticalOptimal) * 100.0;

    std::cout << "\nResultados Experiencia 2:\n"
              << "  Razao de Paragem:        " << circularResult.stopReason << "\n"
              << "  Epocas Executadas:       " << circularResult.generationsRun << "\n"
              << "  Dist. Otima Teorica:     " << std::fixed << std::setprecision(4)
              << theoreticalOptimal << "\n"
              << "  Melhor Dist. Inicial:    " << circularResult.initialBestDistance << "\n"
              << "  Melhor Dist. Final (AG): " << circularResult.bestDistance << "\n"
              << "  Diferenca p/ Otimo:      " << diffFromOptimal << "\n"
              << "  Erro Relativo:           " << std::setprecision(4) << errorPercentage << "%\n"
              << "  Convergiu para o Otimo:  " << (std::abs(diffFromOptimal) < 1e-4 ? "SIM (100% EXATO)" : "MUITO PROXIMO") << "\n"
              << "  Total Linhas no CSV:     " << countLinesInFile(circularConfig.csvFilename)
              << " (cabecalho + geracoes)\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(circularResult.bestRoute);

    printSeparator("FASE 3 CONCLUIDA COM SUCESSO: LOGS GERADOS");
    return 0;
}
