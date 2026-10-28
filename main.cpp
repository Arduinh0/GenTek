#include "types.hpp"
#include "distance.hpp"
#include "fitness.hpp"
#include "generators.hpp"
#include "genetic.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>

// ============================================================================
// main.cpp — Fase 5: Menu Interativo, Pipeline Autônoma e Exportação
// ============================================================================

static constexpr double PI = 3.14159265358979323846;

/// Localiza a raiz do projeto (onde residem CMakeLists.txt e gerar_graficos.py),
/// independentemente de o executável ser chamado a partir da pasta 'build' ou da raiz.
std::filesystem::path findProjectRoot(const char* argv0 = nullptr) {
    // 1. Tentar a partir do diretório de trabalho atual subindo a árvore
    std::filesystem::path p = std::filesystem::current_path();
    for (int i = 0; i < 5; ++i) {
        if (std::filesystem::exists(p / "CMakeLists.txt") && 
            (std::filesystem::exists(p / "src") || std::filesystem::exists(p / "gerar_graficos.py"))) {
            return p;
        }
        if (!p.has_parent_path() || p == p.parent_path()) break;
        p = p.parent_path();
    }
    // 2. Tentar a partir da localização do executável (argv[0])
    if (argv0) {
        std::error_code ec;
        std::filesystem::path execP = std::filesystem::absolute(argv0, ec).parent_path();
        for (int i = 0; i < 5; ++i) {
            if (std::filesystem::exists(execP / "CMakeLists.txt") && 
                (std::filesystem::exists(execP / "src") || std::filesystem::exists(execP / "gerar_graficos.py"))) {
                return execP;
            }
            if (!execP.has_parent_path() || execP == execP.parent_path()) break;
            execP = execP.parent_path();
        }
    }
    return std::filesystem::current_path();
}

/// Imprime uma rota formatada (com resumo caso tenha mais de 35 cidades).
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
    std::cout << "\n" << std::string(72, '=') << "\n"
              << title << "\n"
              << std::string(72, '=') << "\n\n";
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

/// Exporta coordenadas de pontos para CSV (permite ao Python plotar as rotas).
void exportCitiesCSV(const std::string& filename, const std::vector<Point>& cities) {
    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return;
    file << "Cidade,X,Y\n";
    for (size_t i = 0; i < cities.size(); ++i) {
        file << i << "," << std::fixed << std::setprecision(6)
             << cities[i].x << "," << cities[i].y << "\n";
    }
}

int main(int argc, char* argv[]) {
    // 1. Garantir que o diretório de trabalho é sempre a raiz do projeto
    const std::filesystem::path rootDir = findProjectRoot(argc > 0 ? argv[0] : nullptr);
    std::filesystem::current_path(rootDir);

    printSeparator("ALGORITMO GENETICO - TSP 2D (FASE 5: PIPELINE 100% AUTONOMA)");

    // 2. Garantir diretório 'resultados' na raiz do projeto
    const std::filesystem::path outputDir = "resultados";
    std::filesystem::create_directories(outputDir);
    std::cout << "Raiz do projeto identificada: " << std::filesystem::current_path().string() << "\n";
    std::cout << "Diretorio de resultados:      " << std::filesystem::absolute(outputDir).string() << "\n\n";

    // 3. Menu Interativo via std::cin
    std::cout << "================== CONFIGURACAO DOS CENARIOS ==================\n";
    
    // Cenário 1: Aleatório
    int nRandom = 30;
    std::cout << "1. Cenario Aleatorio:\n";
    std::cout << "   Quantidade de cidades (padrao: 30): ";
    if (!(std::cin >> nRandom) || nRandom <= 0) {
        nRandom = 30;
    }

    // Cenário 2: Circular Padrão
    int nCircular = 30;
    double radius30 = 40.0;
    std::cout << "\n2. Cenario Circular Padrao:\n";
    std::cout << "   Quantidade de cidades (padrao: 30): ";
    if (!(std::cin >> nCircular) || nCircular <= 0) {
        nCircular = 30;
    }
    std::cout << "   Raio da circunferencia (padrao: 40.0): ";
    if (!(std::cin >> radius30) || radius30 <= 0.0) {
        radius30 = 40.0;
    }

    // Cenário 3: Circular Bónus
    int nBonus = 100;
    double radius100 = 100.0;
    std::cout << "\n3. Cenario Circular Bonus (Alta Carga):\n";
    std::cout << "   Quantidade de cidades (padrao: 100): ";
    if (!(std::cin >> nBonus) || nBonus <= 0) {
        nBonus = 100;
    }
    std::cout << "   Raio da circunferencia (padrao: 100.0): ";
    if (!(std::cin >> radius100) || radius100 <= 0.0) {
        radius100 = 100.0;
    }
    std::cout << "===============================================================\n";

    // ========================================================================
    // EXPERIÊNCIA 1: Cenário Aleatório
    // ========================================================================
    printSeparator("EXPERIENCIA 1: Cenario Aleatorio (N = " + std::to_string(nRandom) + ")");

    const unsigned randomScenarioSeed = 42;
    const auto randomCities = generateRandomPoints(nRandom, randomScenarioSeed);
    const auto randomDist = computeDistanceMatrix(randomCities);

    exportCitiesCSV((outputDir / "cidades_aleatorio.csv").string(), randomCities);

    GAConfig randomConfig;
    randomConfig.populationSize  = (nRandom > 50) ? 500 : 300;
    randomConfig.tournamentSize  = 3;
    randomConfig.crossoverRate   = 0.85;
    randomConfig.mutationRate    = 0.05;
    randomConfig.eliteCount      = 2;
    randomConfig.maxGenerations  = 2000;
    randomConfig.stagnationLimit = 400;
    randomConfig.seed            = 12345;
    randomConfig.csvFilename     = (outputDir / "log_aleatorio.csv").string();

    std::cout << "Parametros do AG:\n"
              << "  Cidades:       " << nRandom << "\n"
              << "  Populacao:     " << randomConfig.populationSize << "\n"
              << "  Cruzamento:    " << randomConfig.crossoverRate * 100 << "%\n"
              << "  Mutacao:       " << randomConfig.mutationRate * 100 << "%\n"
              << "  Elitismo:      " << randomConfig.eliteCount << "\n"
              << "  Max Geracoes:  " << randomConfig.maxGenerations << "\n"
              << "  Estagnacao:    " << randomConfig.stagnationLimit << "\n"
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
              << "  Total Linhas no CSV:   " << countLinesInFile(randomConfig.csvFilename) << "\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(randomResult.bestRoute);

    // ========================================================================
    // EXPERIÊNCIA 2: Cenário Circular Padrão
    // ========================================================================
    printSeparator("EXPERIENCIA 2: Cenario Circular (N = " + std::to_string(nCircular) +
                   ", Raio = " + std::to_string(static_cast<int>(radius30)) + ")");

    const auto circularCities = generateCircularPoints(nCircular, radius30);
    const auto circularDist = computeDistanceMatrix(circularCities);

    exportCitiesCSV((outputDir / "cidades_circular.csv").string(), circularCities);

    const double theoreticalOptimal30 = 2.0 * nCircular * radius30 * std::sin(PI / nCircular);

    GAConfig circularConfig;
    circularConfig.populationSize  = (nCircular > 50) ? 500 : 300;
    circularConfig.tournamentSize  = 3;
    circularConfig.crossoverRate   = 0.85;
    circularConfig.mutationRate    = 0.05;
    circularConfig.eliteCount      = 2;
    circularConfig.maxGenerations  = 2000;
    circularConfig.stagnationLimit = 400;
    circularConfig.seed            = 54321;
    circularConfig.csvFilename     = (outputDir / "log_circular.csv").string();

    std::cout << "Parametros do AG:\n"
              << "  Cidades:               " << nCircular << "\n"
              << "  Raio:                  " << radius30 << "\n"
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
              << "  Melhor Dist. Final (AG): " << circularResult.bestDistance << "\n"
              << "  Diferenca p/ Otimo:      " << diff30 << "\n"
              << "  Erro Relativo:           " << std::setprecision(4)
              << ((diff30 / theoreticalOptimal30) * 100.0) << "%\n"
              << "  Convergiu para o Otimo:  " << (std::abs(diff30) < 1e-4 ? "SIM (100% EXATO)" : "MUITO PROXIMO") << "\n"
              << "  Total Linhas no CSV:     " << countLinesInFile(circularConfig.csvFilename) << "\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(circularResult.bestRoute);

    // ========================================================================
    // EXPERIÊNCIA 3: Cenário Circular Bónus (Alta Carga)
    // ========================================================================
    printSeparator("EXPERIENCIA 3 (BONUS): Cenario Circular de Alta Carga (N = " +
                   std::to_string(nBonus) + ", Raio = " + std::to_string(static_cast<int>(radius100)) + ")");

    const auto bonusCities = generateCircularPoints(nBonus, radius100);
    const auto bonusDist = computeDistanceMatrix(bonusCities);

    exportCitiesCSV((outputDir / "cidades_bonus_100.csv").string(), bonusCities);

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

    std::cout << "Parametros do AG:\n"
              << "  Cidades:               " << nBonus << "\n"
              << "  Raio:                  " << radius100 << "\n"
              << "  Dist. Otima Teorica:   " << std::fixed << std::setprecision(4)
              << theoreticalOptimal100 << "\n"
              << "  Populacao:             " << bonusConfig.populationSize << "\n"
              << "  Ficheiro CSV:          " << bonusConfig.csvFilename << "\n\n"
              << "Evolucao:\n";

    const GAResult bonusResult = runGA(bonusCities, bonusDist, bonusConfig);

    const double diff100 = bonusResult.bestDistance - theoreticalOptimal100;
    const double errorPerc100 = (diff100 / theoreticalOptimal100) * 100.0;

    std::cout << "\nResultados Experiencia 3 (Bonus):\n"
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
              << "  Melhor Dist. Final (AG): " << bonusResult.bestDistance << "\n"
              << "  Diferenca p/ Otimo:      " << diff100 << "\n"
              << "  Erro Relativo:           " << std::setprecision(4) << errorPerc100 << "%\n"
              << "  Total Linhas no CSV:     " << countLinesInFile(bonusConfig.csvFilename) << "\n\n"
              << "  Melhor Rota Encontrada:\n";
    printRoute(bonusResult.bestRoute);

    // ========================================================================
    // 4. AUTOMAÇÃO: CHAMADA DO SCRIPT PYTHON
    // ========================================================================
    printSeparator("AUTOMACAO: GERACAO DE GRAFICOS (PYTHON)");
    std::cout << "Executando gerar_graficos.py automaticamente na raiz do projeto...\n\n";

    // Executa no Windows e Unix com fallback
    const int pyStatus = std::system("python gerar_graficos.py || python3 gerar_graficos.py");
    if (pyStatus == 0) {
        std::cout << "\n[OK] Pipeline completa finalizada com sucesso! Graficos atualizados em 'graficos/'.\n";
    } else {
        std::cerr << "\n[AVISO] O script Python retornou codigo: " << pyStatus
                  << ". Certifique-se de que python com pandas e matplotlib estao instalados.\n";
    }

    printSeparator("EXECUCAO CONCLUIDA COM SUCESSO!");
    return 0;
}
