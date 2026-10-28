#include "types.hpp"
#include "distance.hpp"
#include "fitness.hpp"
#include "generators.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>

// ============================================================================
// main.cpp — Driver de Testes da Fase 1
// Valida: geração de cenários, matriz de distâncias, rota e fitness.
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

/// Imprime a matriz de distâncias formatada.
void printDistMatrix(const DistMatrix& matrix) {
    const int n = static_cast<int>(matrix.size());
    const int colWidth = 9;

    // Cabeçalho
    std::cout << std::string(colWidth, ' ');
    for (int j = 0; j < n; ++j) {
        std::cout << std::setw(colWidth) << j;
    }
    std::cout << "\n";

    // Linhas
    for (int i = 0; i < n; ++i) {
        std::cout << std::setw(colWidth) << i;
        for (int j = 0; j < n; ++j) {
            std::cout << std::setw(colWidth) << std::fixed
                      << std::setprecision(2) << matrix[i][j];
        }
        std::cout << "\n";
    }
}

/// Separador visual entre seções.
void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n"
              << title << "\n"
              << std::string(60, '=') << "\n\n";
}

int main() {
    // ========================================================================
    // TESTE 1 — Pontos Aleatórios (5 cidades, seed fixa para reprodutibilidade)
    // ========================================================================
    printSeparator("TESTE 1: Pontos Aleatorios (5 cidades, seed = 42)");

    const auto randomCities = generateRandomPoints(5, 42);
    printCities(randomCities);

    std::cout << "\nMatriz de Distancias:\n";
    const auto randomDist = computeDistanceMatrix(randomCities);
    printDistMatrix(randomDist);

    // Rota sequencial [0, 1, 2, 3, 4]
    Individual sequentialRoute(5);
    std::iota(sequentialRoute.begin(), sequentialRoute.end(), 0);

    const double randomRouteDist = routeDistance(sequentialRoute, randomDist);
    const double randomFit = fitness(sequentialRoute, randomDist);

    std::cout << "\nRota sequencial [0 -> 1 -> 2 -> 3 -> 4 -> 0]:\n"
              << "  Distancia total: " << std::fixed << std::setprecision(4)
              << randomRouteDist << "\n"
              << "  Fitness:         " << std::scientific << std::setprecision(6)
              << randomFit << "\n";

    // ========================================================================
    // TESTE 2 — Pontos Circulares (8 cidades, raio 40)
    // ========================================================================
    printSeparator("TESTE 2: Pontos Circulares (8 cidades, raio = 40)");

    const int nCircular = 8;
    const double radius = 40.0;

    const auto circularCities = generateCircularPoints(nCircular, radius);
    printCities(circularCities);

    std::cout << "\nMatriz de Distancias:\n";
    const auto circularDist = computeDistanceMatrix(circularCities);
    printDistMatrix(circularDist);

    // Rota sequencial [0, 1, ..., 7] — deve ser a solução ótima no círculo
    Individual circularRoute(nCircular);
    std::iota(circularRoute.begin(), circularRoute.end(), 0);

    const double circularRouteDist = routeDistance(circularRoute, circularDist);

    // Distância teórica do perímetro: 2 * N * r * sin(π / N)
    const double theoreticalPerimeter =
        2.0 * nCircular * radius * std::sin(PI / nCircular);

    const double circularFit = fitness(circularRoute, circularDist);

    std::cout << "\nRota sequencial [0 -> 1 -> ... -> 7 -> 0]:\n"
              << "  Distancia calculada: " << std::fixed << std::setprecision(4)
              << circularRouteDist << "\n"
              << "  Distancia teorica:   " << theoreticalPerimeter << "\n";

    // Verificação: diferença absoluta deve ser desprezível (< 1e-6)
    const double diff = std::abs(circularRouteDist - theoreticalPerimeter);
    if (diff < 1e-6) {
        std::cout << "  Verificacao:         PASSOU (diff = "
                  << std::scientific << diff << ")\n";
    } else {
        std::cout << "  Verificacao:         FALHOU (diff = "
                  << std::scientific << diff << ")\n";
    }

    std::cout << "  Fitness:             " << std::scientific
              << std::setprecision(6) << circularFit << "\n";

    // ========================================================================
    // TESTE 3 — Validações de Entrada (exceções)
    // ========================================================================
    printSeparator("TESTE 3: Validacoes de Entrada");

    // Teste: n <= 0 em generateRandomPoints
    try {
        generateRandomPoints(0);
        std::cout << "  [FALHOU] generateRandomPoints(0) nao lancou excecao.\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "  [PASSOU] generateRandomPoints(0): " << e.what() << "\n";
    }

    // Teste: n <= 0 em generateCircularPoints
    try {
        generateCircularPoints(-1, 10.0);
        std::cout << "  [FALHOU] generateCircularPoints(-1, 10) nao lancou excecao.\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "  [PASSOU] generateCircularPoints(-1, 10): " << e.what() << "\n";
    }

    // Teste: radius <= 0 em generateCircularPoints
    try {
        generateCircularPoints(5, -3.0);
        std::cout << "  [FALHOU] generateCircularPoints(5, -3) nao lancou excecao.\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "  [PASSOU] generateCircularPoints(5, -3): " << e.what() << "\n";
    }

    // ========================================================================
    // TESTE 4 — Caso Degenerado (fitness com rota vazia)
    // ========================================================================
    printSeparator("TESTE 4: Caso Degenerado (rota vazia)");

    const Individual emptyRoute;
    const DistMatrix emptyDist;
    const double emptyFit = fitness(emptyRoute, emptyDist);

    std::cout << "  Distancia da rota vazia: "
              << routeDistance(emptyRoute, emptyDist) << "\n"
              << "  Fitness da rota vazia:   " << emptyFit
              << " (esperado: DBL_MAX)\n";

    // ========================================================================
    std::cout << "\n" << std::string(60, '=') << "\n"
              << "Fase 1 concluida com sucesso! Todos os testes executados.\n"
              << std::string(60, '=') << "\n";

    return 0;
}
