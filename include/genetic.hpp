#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "types.hpp"
#include <random>
#include <string>
#include <utility>
#include <vector>

// ============================================================================
// genetic.hpp — Operadores Genéticos e Ciclo de Evolução para AG-TSP
// ============================================================================

// ----------------------------------------------------------------------------
// Estruturas de Configuração e Resultado
// ----------------------------------------------------------------------------

/// Hiperparâmetros do Algoritmo Genético.
struct GAConfig {
    int    populationSize   = 200;    ///< Tamanho da população
    int    tournamentSize   = 3;      ///< k indivíduos no torneio de seleção
    double crossoverRate    = 0.85;   ///< Probabilidade de cruzamento (Pc)
    double mutationRate     = 0.03;   ///< Probabilidade de mutação (Pm)
    int    eliteCount       = 2;      ///< Nº de elites que transitam intactos
    int    maxGenerations   = 1500;   ///< Critério de paragem: máx gerações
    int    stagnationLimit  = 250;    ///< Critério de paragem: gerações sem melhoria
    unsigned seed           = 0;      ///< Seed do RNG (0 = aleatória via random_device)
};

/// Resultado da execução do AG.
struct GAResult {
    Individual  bestRoute;            ///< Melhor rota encontrada
    double      bestDistance;         ///< Distância da melhor rota
    double      bestFitness;          ///< Fitness da melhor rota
    int         generationsRun;       ///< Nº total de gerações executadas
    double      initialBestDistance;  ///< Melhor distância na população inicial
    std::string stopReason;           ///< "MAX_GENERATIONS" ou "STAGNATION"
};

/// Indivíduo com cache da avaliação (distância e fitness pré-calculados).
/// Evita recalcular routeDistance em cada comparação do sort e do torneio.
struct EvaluatedIndividual {
    Individual route;
    double     distance;
    double     fitValue;
};

// ----------------------------------------------------------------------------
// Operadores Genéticos
// ----------------------------------------------------------------------------

/// Gera uma população de 'popSize' indivíduos, cada um sendo uma permutação
/// aleatória de [0, 1, ..., numCities-1] obtida via std::shuffle.
/// Cada indivíduo já é avaliado (distance e fitness pré-calculados).
/// @param numCities Número de cidades.
/// @param popSize   Tamanho da população.
/// @param dist      Matriz de distâncias pré-calculada.
/// @param rng       Gerador pseudo-aleatório (Mersenne Twister).
/// @return Vetor de EvaluatedIndividual com 'popSize' elementos.
std::vector<EvaluatedIndividual> initializePopulation(int numCities, int popSize,
                                                      const DistMatrix& dist,
                                                      std::mt19937& rng);

/// Seleção por Torneio: sorteia 'tournamentSize' candidatos aleatórios da
/// população e devolve uma referência ao que tiver menor distância.
/// @param population     População atual (já avaliada).
/// @param tournamentSize Número de candidatos no torneio (k).
/// @param rng            Gerador pseudo-aleatório.
/// @return Referência constante ao melhor candidato do torneio.
const EvaluatedIndividual& tournamentSelection(
    const std::vector<EvaluatedIndividual>& population,
    int tournamentSize,
    std::mt19937& rng);

/// Order Crossover (OX): recebe dois pais e produz dois filhos, garantindo
/// que cada filho é uma permutação válida (sem cidades duplicadas/omitidas).
/// @param parent1 Primeiro pai.
/// @param parent2 Segundo pai.
/// @param rng     Gerador pseudo-aleatório.
/// @return Par de filhos (child1, child2).
std::pair<Individual, Individual> orderCrossover(const Individual& parent1,
                                                 const Individual& parent2,
                                                 std::mt19937& rng);

/// Mutação por Inversão: com probabilidade 'mutationRate', sorteia dois pontos
/// de corte e inverte a subsequência entre eles (inclusive). Modifica in-place.
/// @param individual   Indivíduo a mutar.
/// @param mutationRate Probabilidade de mutação (Pm).
/// @param rng          Gerador pseudo-aleatório.
void inversionMutation(Individual& individual, double mutationRate,
                       std::mt19937& rng);

// ----------------------------------------------------------------------------
// Ciclo de Evolução
// ----------------------------------------------------------------------------

/// Executa o Algoritmo Genético completo: inicializa a população, aplica
/// seleção, cruzamento, mutação e elitismo ao longo das gerações até um dos
/// critérios de paragem ser atingido (máx gerações ou estagnação).
/// @param cities Vetor de pontos (cidades).
/// @param dist   Matriz de distâncias pré-calculada.
/// @param config Hiperparâmetros do AG.
/// @return Estrutura GAResult com o melhor resultado encontrado.
GAResult runGA(const std::vector<Point>& cities,
               const DistMatrix& dist,
               const GAConfig& config);

#endif // GENETIC_HPP
