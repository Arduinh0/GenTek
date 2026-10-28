#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "types.hpp"
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

// ============================================================================
// genetic.hpp — Operadores Genéticos, Telemetria e Ciclo de Evolução
// ============================================================================

// ----------------------------------------------------------------------------
// Estruturas de Telemetria e Dados
// ----------------------------------------------------------------------------

/// Registos de telemetria de uma geração (época).
struct GenerationData {
    int        epoch;           ///< Número da geração (0 = população inicial, 1, 2, ...)
    double     bestDistance;    ///< Menor distância da população nesta geração
    double     avgDistance;     ///< Distância média da população nesta geração
    Individual bestRoute;       ///< Cópia da melhor rota nesta geração
};

/// Formata uma rota como string de índices de cidades separados por traço ("0-5-12-3...").
std::string formatRoute(const Individual& route);

/// Indivíduo com cache da avaliação (distância e fitness pré-calculados).
/// Evita recalcular routeDistance em cada comparação do sort e do torneio.
struct EvaluatedIndividual {
    Individual route;
    double     distance;
    double     fitValue;
};

// ----------------------------------------------------------------------------
// Estruturas de Configuração e Resultado
// ----------------------------------------------------------------------------

/// Hiperparâmetros do Algoritmo Genético e opções de telemetria.
struct GAConfig {
    int    populationSize   = 200;    ///< Tamanho da população
    int    tournamentSize   = 3;      ///< k indivíduos no torneio de seleção
    double crossoverRate    = 0.85;   ///< Probabilidade de cruzamento (Pc)
    double mutationRate     = 0.03;   ///< Probabilidade de mutação (Pm)
    int    eliteCount       = 2;      ///< Nº de elites que transitam intactos
    int    maxGenerations   = 1500;   ///< Critério de paragem: máx gerações
    int    stagnationLimit  = 250;    ///< Critério de paragem: gerações sem melhoria
    unsigned seed           = 0;      ///< Seed do RNG (0 = aleatória via random_device)

    // Opções de Telemetria
    std::string csvFilename = "";     ///< Se preenchido, exporta CSV (Epoca,MelhorDistancia,DistanciaMedia,MelhorRota)
    std::function<void(const GenerationData&)> onGeneration = nullptr; ///< Callback por geração
};

/// Resultado da execução do AG.
struct GAResult {
    Individual                  bestRoute;            ///< Melhor rota encontrada
    double                      bestDistance;         ///< Distância da melhor rota
    double                      bestFitness;          ///< Fitness da melhor rota
    int                         generationsRun;       ///< Nº total de gerações executadas
    double                      initialBestDistance;  ///< Melhor distância na população inicial
    std::string                 stopReason;           ///< "MAX_GENERATIONS" ou "STAGNATION"
    std::vector<GenerationData> history;              ///< Histórico completo de telemetria época a época
    double                      executionTimeMs;      ///< Tempo total de execução do GA em milissegundos (std::chrono)
};

// ----------------------------------------------------------------------------
// Operadores Genéticos
// ----------------------------------------------------------------------------

/// Gera uma população de 'popSize' indivíduos, cada um sendo uma permutação
/// aleatória de [0, 1, ..., numCities-1] obtida via std::shuffle.
/// Cada indivíduo já é avaliado (distance e fitness pré-calculados).
std::vector<EvaluatedIndividual> initializePopulation(int numCities, int popSize,
                                                      const DistMatrix& dist,
                                                      std::mt19937& rng);

/// Seleção por Torneio: sorteia 'tournamentSize' candidatos aleatórios da
/// população e devolve uma referência ao que tiver menor distância.
const EvaluatedIndividual& tournamentSelection(
    const std::vector<EvaluatedIndividual>& population,
    int tournamentSize,
    std::mt19937& rng);

/// Order Crossover (OX): recebe dois pais e produz dois filhos, garantindo
/// que cada filho é uma permutação válida (sem cidades duplicadas/omitidas).
std::pair<Individual, Individual> orderCrossover(const Individual& parent1,
                                                 const Individual& parent2,
                                                 std::mt19937& rng);

/// Mutação por Inversão: com probabilidade 'mutationRate', sorteia dois pontos
/// de corte e inverte a subsequência entre eles (inclusive). Modifica in-place.
void inversionMutation(Individual& individual, double mutationRate,
                       std::mt19937& rng);

// ----------------------------------------------------------------------------
// Ciclo de Evolução
// ----------------------------------------------------------------------------

/// Executa o Algoritmo Genético completo com telemetria (exportação CSV, callbacks e medição de tempo).
GAResult runGA(const std::vector<Point>& cities,
               const DistMatrix& dist,
               const GAConfig& config);

#endif // GENETIC_HPP
