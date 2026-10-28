#include "types.hpp"
#include "distance.hpp"
#include "fitness.hpp"
#include "generators.hpp"
#include "genetic.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>

// ============================================================================
// main.cpp — Fase 4: Cenário Bónus (Alta Volumetria), Telemetria de Tempo
//                     e Organização de Ficheiros com <filesystem>
// ============================================================================

static constexpr double PI = 3.14159265358979323846;

/// Imprime uma rota formatada (com resumo caso tenha mais de 30 cidades).
void printRoute(const Individual& route) {
    const int n = static_cast<int>(route.size());
    std::cout << "  [";
    if (n <= 35) {
        for (int i = 0; i < n; ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << route[i];
        }
        std::cout << " -> " << route[0] << "]\n";
    } else {
        // Exibição compacta para rotas longas (ex: N=100)
        for (int i = 0; i < 10; ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << route[i];
        }
        std::cout << " -> ... (" << (n - 20) << " cidades intermediarias) ...";
        for (int i = n - 10; i < n; ++i) {
            std::cout << " -> " << route[i];
        }
        std::cout << " -> " << route[0] << "]\n";
    }
}

/// Separador visual entre seções.
void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << "\n"
              << title << "\n"
              << std::string(70, '=') << "\n\n";
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
    printSeparator("FASE 4: ALTA VOLUMETRIA, TELEMETRIA DE TEMPO E ORGANIZACAO");

    // 1. Organização Dinâmica de Diretórios via <filesystem>
    const std::filesystem::path outputDir = "resultados";
    std::filesystem::create_directories(outputDir);
    std::cout << "Diretorio de saida garantido: " << std::filesystem::absolute(outputDir).string() << "\n";

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
    randomConfig.csvFilename     = (outputDir / "log_aleatorio.csv").string();

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
              << "  Tempo de Execucao:     " << std::fixed << std::setprecision(2)
              << randomResult.executionTimeMs << " ms ("
              << std::setprecision(4) << (randomResult.executionTimeMs / 1000.0) << " s)\n"
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
              << "  Total Linhas no CSV:   " << countLinesInFile(randomConfig.csvFilename) << "\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(randomResult.bestRoute);

    // ========================================================================
    // EXPERIÊNCIA 2: Cenário Circular Benchmark (N = 30 Cidades, Raio = 40)
    // ========================================================================
    printSeparator("EXPERIENCIA 2: Cenario Circular Benchmark (N = 30 cidades, Raio = 40)");

    const int nCircular = 30;
    const double radius30 = 40.0;

    const auto circularCities = generateCircularPoints(nCircular, radius30);
    const auto circularDist = computeDistanceMatrix(circularCities);

    const double theoreticalOptimal30 = 2.0 * nCircular * radius30 * std::sin(PI / nCircular);

    GAConfig circularConfig;
    circularConfig.populationSize  = 300;
    circularConfig.tournamentSize  = 3;
    circularConfig.crossoverRate   = 0.85;
    circularConfig.mutationRate    = 0.05;
    circularConfig.eliteCount      = 2;
    circularConfig.maxGenerations  = 2000;
    circularConfig.stagnationLimit = 400;
    circularConfig.seed            = 54321;
    circularConfig.csvFilename     = (outputDir / "log_circular.csv").string();

    std::cout << "Configuracao do AG (Benchmark Circular 30):\n"
              << "  Cidades:               " << nCircular << "\n"
              << "  Raio do Circulo:       " << radius30 << "\n"
              << "  Dist. Otima Teorica:   " << std::fixed << std::setprecision(4)
              << theoreticalOptimal30 << "\n"
              << "  Populacao:             " << circularConfig.populationSize << "\n"
              << "  Ficheiro CSV:          " << circularConfig.csvFilename << "\n\n"
              << "Evolucao:\n";

    const GAResult circularResult = runGA(circularCities, circularDist, circularConfig);

    const double diff30 = circularResult.bestDistance - theoreticalOptimal30;

    std::cout << "\nResultados Experiencia 2:\n"
              << "  Tempo de Execucao:       " << std::fixed << std::setprecision(2)
              << circularResult.executionTimeMs << " ms ("
              << std::setprecision(4) << (circularResult.executionTimeMs / 1000.0) << " s)\n"
              << "  Razao de Paragem:        " << circularResult.stopReason << "\n"
              << "  Epocas Executadas:       " << circularResult.generationsRun << "\n"
              << "  Dist. Otima Teorica:     " << std::fixed << std::setprecision(4)
              << theoreticalOptimal30 << "\n"
              << "  Melhor Dist. Inicial:    " << circularResult.initialBestDistance << "\n"
              << "  Melhor Dist. Final (AG): " << circularResult.bestDistance << "\n"
              << "  Diferenca p/ Otimo:      " << diff30 << "\n"
              << "  Erro Relativo:           " << std::setprecision(4)
              << ((diff30 / theoreticalOptimal30) * 100.0) << "%\n"
              << "  Convergiu para o Otimo:  " << (std::abs(diff30) < 1e-4 ? "SIM (100% EXATO)" : "MUITO PROXIMO") << "\n"
              << "  Total Linhas no CSV:     " << countLinesInFile(circularConfig.csvFilename) << "\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(circularResult.bestRoute);

    // ========================================================================
    // EXPERIÊNCIA 3 (BÓNUS): Cenário Circular de Alta Carga (N = 100, Raio = 100)
    // ========================================================================
    printSeparator("EXPERIENCIA 3 (BONUS): Alta Carga - Cenario Circular (N = 100, Raio = 100)");

    const int nBonus = 100;
    const double radius100 = 100.0;

    const auto bonusCities = generateCircularPoints(nBonus, radius100);
    const auto bonusDist = computeDistanceMatrix(bonusCities);

    // Perímetro teórico para N=100, R=100: 2 * 100 * 100 * sin(pi / 100) ≈ 627.9052
    const double theoreticalOptimal100 = 2.0 * nBonus * radius100 * std::sin(PI / nBonus);

    GAConfig bonusConfig;
    bonusConfig.populationSize  = 500;
    bonusConfig.tournamentSize  = 3;
    bonusConfig.crossoverRate   = 0.85;
    bonusConfig.mutationRate    = 0.06;
    bonusConfig.eliteCount      = 3;
    bonusConfig.maxGenerations  = 4000;
    bonusConfig.stagnationLimit = 700;
    bonusConfig.seed            = 999;
    bonusConfig.csvFilename     = (outputDir / "log_bonus_100.csv").string();

    std::cout << "Configuracao do AG (Cenario Bonus N=100):\n"
              << "  Cidades (Genes):       " << nBonus << " (Espaco de Busca: ~9.33e+157 rotas)\n"
              << "  Raio do Circulo:       " << radius100 << "\n"
              << "  Dist. Otima Teorica:   " << std::fixed << std::setprecision(4)
              << theoreticalOptimal100 << "\n"
              << "  Populacao:             " << bonusConfig.populationSize << "\n"
              << "  Torneio (k):           " << bonusConfig.tournamentSize << "\n"
              << "  Cruzamento (OX):       " << bonusConfig.crossoverRate * 100 << "%\n"
              << "  Mutacao (Inversao):    " << bonusConfig.mutationRate * 100 << "%\n"
              << "  Elitismo:              " << bonusConfig.eliteCount << " individuos\n"
              << "  Max Geracoes:          " << bonusConfig.maxGenerations << "\n"
              << "  Limite de Estagnacao:  " << bonusConfig.stagnationLimit << " geracoes\n"
              << "  Ficheiro de Saida:     " << bonusConfig.csvFilename << "\n\n"
              << "Evolucao:\n";

    const GAResult bonusResult = runGA(bonusCities, bonusDist, bonusConfig);

    const double diff100 = bonusResult.bestDistance - theoreticalOptimal100;
    const double errorPerc100 = (diff100 / theoreticalOptimal100) * 100.0;

    std::cout << "\nResultados Experiencia 3 (Bonus N=100):\n"
              << "  ================ TELEMETRIA DE TEMPO ================\n"
              << "  Tempo Total de Execucao: " << std::fixed << std::setprecision(2)
              << bonusResult.executionTimeMs << " ms ("
              << std::setprecision(4) << (bonusResult.executionTimeMs / 1000.0) << " segundos)\n"
              << "  Tempo Medio por Epoca:   " << std::fixed << std::setprecision(3)
              << (bonusResult.executionTimeMs / (bonusResult.generationsRun + 1)) << " ms/epoca\n"
              << "  =====================================================\n"
              << "  Razao de Paragem:        " << bonusResult.stopReason << "\n"
              << "  Epocas Executadas:       " << bonusResult.generationsRun << "\n"
              << "  Dist. Otima Teorica:     " << std::fixed << std::setprecision(4)
              << theoreticalOptimal100 << "\n"
              << "  Melhor Dist. Inicial:    " << bonusResult.initialBestDistance << "\n"
              << "  Melhor Dist. Final (AG): " << bonusResult.bestDistance << "\n"
              << "  Diferenca p/ Otimo:      " << diff100 << "\n"
              << "  Erro Relativo:           " << std::setprecision(4) << errorPerc100 << "%\n"
              << "  Proximidade do Otimo:    " << std::setprecision(2) << (100.0 - errorPerc100) << "%\n"
              << "  Total Linhas no CSV:     " << countLinesInFile(bonusConfig.csvFilename) << "\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(bonusResult.bestRoute);

    printSeparator("FASE 4 CONCLUIDA COM SUCESSO: RESULTADOS GERADOS");
    return 0;
}
