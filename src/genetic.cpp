#include "genetic.hpp"
#include "distance.hpp"
#include "fitness.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

// ============================================================================
// genetic.cpp — Implementação dos Operadores Genéticos, Telemetria e Ciclo
// ============================================================================

// ----------------------------------------------------------------------------
// Formatação de rotas
// ----------------------------------------------------------------------------
std::string formatRoute(const Individual& route) {
    if (route.empty()) {
        return "";
    }
    std::ostringstream oss;
    for (size_t i = 0; i < route.size(); ++i) {
        if (i > 0) {
            oss << '-';
        }
        oss << route[i];
    }
    return oss.str();
}

// ----------------------------------------------------------------------------
// Função auxiliar: avalia um indivíduo e devolve um EvaluatedIndividual
// ----------------------------------------------------------------------------
static EvaluatedIndividual evaluate(const Individual& route,
                                    const DistMatrix& dist) {
    const double d = routeDistance(route, dist);
    const double f = (d == 0.0) ? std::numeric_limits<double>::max() : 1.0 / d;
    return {route, d, f};
}

// ----------------------------------------------------------------------------
// 1. Inicialização da População
// ----------------------------------------------------------------------------
std::vector<EvaluatedIndividual> initializePopulation(const int numCities,
                                                      const int popSize,
                                                      const DistMatrix& dist,
                                                      std::mt19937& rng) {
    // Cria o vetor base {0, 1, ..., N-1}
    Individual base(numCities);
    std::iota(base.begin(), base.end(), 0);

    std::vector<EvaluatedIndividual> population;
    population.reserve(popSize);

    for (int i = 0; i < popSize; ++i) {
        std::shuffle(base.begin(), base.end(), rng);
        population.push_back(evaluate(base, dist));
    }

    return population;
}

// ----------------------------------------------------------------------------
// 2. Seleção por Torneio
// ----------------------------------------------------------------------------
const EvaluatedIndividual& tournamentSelection(
    const std::vector<EvaluatedIndividual>& population,
    const int tournamentSize,
    std::mt19937& rng) {

    std::uniform_int_distribution<int> indexDist(0,
        static_cast<int>(population.size()) - 1);

    // Primeiro candidato
    int bestIdx = indexDist(rng);

    // Competir com os restantes k-1 candidatos
    for (int t = 1; t < tournamentSize; ++t) {
        const int candidate = indexDist(rng);
        // Menor distância = melhor indivíduo
        if (population[candidate].distance < population[bestIdx].distance) {
            bestIdx = candidate;
        }
    }

    return population[bestIdx];
}

// ----------------------------------------------------------------------------
// 3. Cruzamento — Order Crossover (OX)
// ----------------------------------------------------------------------------
std::pair<Individual, Individual> orderCrossover(const Individual& parent1,
                                                 const Individual& parent2,
                                                 std::mt19937& rng) {
    const int n = static_cast<int>(parent1.size());

    // Sorteia dois pontos de corte distintos
    std::uniform_int_distribution<int> cutDist(0, n - 1);
    int c1 = cutDist(rng);
    int c2 = cutDist(rng);
    while (c2 == c1) {
        c2 = cutDist(rng);
    }
    if (c1 > c2) {
        std::swap(c1, c2);
    }

    // Função lambda que gera um filho dado (doador do segmento, doador do preenchimento)
    auto buildChild = [&](const Individual& segmentDonor,
                          const Individual& fillDonor) -> Individual {
        Individual child(n, -1);

        // 1. Copiar o segmento [c1..c2] do segmentDonor
        std::vector<bool> inSegment(n, false);
        for (int i = c1; i <= c2; ++i) {
            child[i] = segmentDonor[i];
            inSegment[segmentDonor[i]] = true;
        }

        // 2. Preencher as posições restantes com cidades do fillDonor,
        //    na ordem circular a partir de c2+1, saltando as já presentes
        int fillPos = (c2 + 1) % n;  // Posição no child a preencher
        int readPos = (c2 + 1) % n;  // Posição no fillDonor a ler

        for (int count = 0; count < n - (c2 - c1 + 1); ++count) {
            // Avançar no fillDonor até encontrar uma cidade não presente no segmento
            while (inSegment[fillDonor[readPos]]) {
                readPos = (readPos + 1) % n;
            }
            child[fillPos] = fillDonor[readPos];
            fillPos = (fillPos + 1) % n;
            readPos = (readPos + 1) % n;
        }

        return child;
    };

    // child1: segmento de parent1, preenchimento de parent2
    // child2: segmento de parent2, preenchimento de parent1
    return {buildChild(parent1, parent2), buildChild(parent2, parent1)};
}

// ----------------------------------------------------------------------------
// 4. Mutação — Inversão
// ----------------------------------------------------------------------------
void inversionMutation(Individual& individual, const double mutationRate,
                       std::mt19937& rng) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);

    if (prob(rng) >= mutationRate) {
        return;  // Não muta
    }

    const int n = static_cast<int>(individual.size());
    if (n < 2) {
        return;  // Impossível inverter com menos de 2 genes
    }

    // Sorteia dois pontos de corte distintos
    std::uniform_int_distribution<int> cutDist(0, n - 1);
    int i = cutDist(rng);
    int j = cutDist(rng);
    while (j == i) {
        j = cutDist(rng);
    }
    if (i > j) {
        std::swap(i, j);
    }

    // Inverte a subsequência [i..j]
    std::reverse(individual.begin() + i, individual.begin() + j + 1);
}

// ----------------------------------------------------------------------------
// 5. Ciclo de Evolução com Telemetria — runGA
// ----------------------------------------------------------------------------
GAResult runGA(const std::vector<Point>& cities,
               const DistMatrix& dist,
               const GAConfig& config) {

    const int numCities = static_cast<int>(cities.size());

    // Inicializar RNG
    std::mt19937 rng(config.seed == 0 ? std::random_device{}() : config.seed);

    // Preparar exportação CSV se configurada
    std::ofstream csvFile;
    if (!config.csvFilename.empty()) {
        csvFile.open(config.csvFilename);
        if (csvFile.is_open()) {
            csvFile << "Epoca,MelhorDistancia,DistanciaMedia,MelhorRota\n";
        } else {
            std::cerr << "  [AVISO] Nao foi possivel abrir o ficheiro de log: "
                      << config.csvFilename << "\n";
        }
    }

    // --- Inicialização da população ---
    auto population = initializePopulation(numCities, config.populationSize,
                                           dist, rng);

    // Ordenar por distância (crescente): population[0] é o melhor
    std::sort(population.begin(), population.end(),
              [](const EvaluatedIndividual& a, const EvaluatedIndividual& b) {
                  return a.distance < b.distance;
              });

    // Estruturas de resultado e telemetria
    std::vector<GenerationData> history;
    history.reserve(config.maxGenerations + 1);

    // Helper para registar telemetria de uma geração
    auto logGeneration = [&](int epoch, const std::vector<EvaluatedIndividual>& pop) {
        // Cálculo em O(P) da distância média através do cache
        double sumDist = 0.0;
        for (const auto& ind : pop) {
            sumDist += ind.distance;
        }
        const double avgDist = sumDist / pop.size();
        const double bestDist = pop[0].distance;

        GenerationData data{epoch, bestDist, avgDist, pop[0].route};
        history.push_back(data);

        // Callback em memória (se configurado)
        if (config.onGeneration) {
            config.onGeneration(data);
        }

        // Streaming direto para CSV
        if (csvFile.is_open()) {
            csvFile << epoch << ","
                    << std::fixed << std::setprecision(4) << bestDist << ","
                    << avgDist << ","
                    << formatRoute(pop[0].route) << "\n";
        }
    };

    // Registar época 0 (população inicial)
    logGeneration(0, population);

    // Melhor global
    EvaluatedIndividual bestEver = population[0];
    const double initialBestDistance = bestEver.distance;
    int stagnationCounter = 0;
    int generationsRun = 0;
    std::string stopReason = "MAX_GENERATIONS";

    std::cout << "  Gen    0: melhor = " << std::fixed
              << std::setprecision(4) << bestEver.distance
              << " | media = " << history[0].avgDistance << " (inicial)\n";

    // --- Ciclo evolutivo ---
    for (int gen = 1; gen <= config.maxGenerations; ++gen) {
        std::vector<EvaluatedIndividual> newPopulation;
        newPopulation.reserve(config.populationSize);

        // 1. Elitismo: copiar os melhores diretamente
        for (int e = 0; e < config.eliteCount && e < config.populationSize; ++e) {
            newPopulation.push_back(population[e]);
        }

        // 2. Gerar o resto da nova população
        std::uniform_real_distribution<double> prob(0.0, 1.0);

        while (static_cast<int>(newPopulation.size()) < config.populationSize) {
            // Seleção por torneio
            const auto& parent1 = tournamentSelection(population,
                                                      config.tournamentSize, rng);
            const auto& parent2 = tournamentSelection(population,
                                                      config.tournamentSize, rng);

            Individual child1, child2;

            // Cruzamento OX (com probabilidade crossoverRate)
            if (prob(rng) < config.crossoverRate) {
                auto [c1, c2] = orderCrossover(parent1.route, parent2.route, rng);
                child1 = std::move(c1);
                child2 = std::move(c2);
            } else {
                child1 = parent1.route;
                child2 = parent2.route;
            }

            // Mutação por inversão
            inversionMutation(child1, config.mutationRate, rng);
            inversionMutation(child2, config.mutationRate, rng);

            // Avaliar e adicionar à nova população
            newPopulation.push_back(evaluate(child1, dist));
            if (static_cast<int>(newPopulation.size()) < config.populationSize) {
                newPopulation.push_back(evaluate(child2, dist));
            }
        }

        // 3. Substituir população e ordenar
        population = std::move(newPopulation);
        std::sort(population.begin(), population.end(),
                  [](const EvaluatedIndividual& a, const EvaluatedIndividual& b) {
                      return a.distance < b.distance;
                  });

        // 4. Registar telemetria da geração
        logGeneration(gen, population);

        // 5. Atualizar melhor global
        const auto& currentBest = population[0];
        if (currentBest.distance < bestEver.distance) {
            bestEver = currentBest;
            stagnationCounter = 0;
        } else {
            ++stagnationCounter;
        }

        generationsRun = gen;

        // 6. Log no terminal a cada 100 gerações
        if (gen % 100 == 0) {
            std::cout << "  Gen " << std::setw(4) << gen
                      << ": melhor = " << std::fixed << std::setprecision(4)
                      << bestEver.distance
                      << " | media = " << history.back().avgDistance
                      << " | estagnacao = " << stagnationCounter << "/" 
                      << config.stagnationLimit << "\n";
        }

        // 7. Critério de paragem por estagnação
        if (stagnationCounter >= config.stagnationLimit) {
            stopReason = "STAGNATION";
            break;
        }
    }

    if (csvFile.is_open()) {
        csvFile.close();
    }

    return GAResult{
        bestEver.route,
        bestEver.distance,
        bestEver.fitValue,
        generationsRun,
        initialBestDistance,
        stopReason,
        std::move(history)
    };
}
