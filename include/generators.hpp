#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include "types.hpp"
#include <vector>

// ============================================================================
// generators.hpp — Geradores de Instâncias (Cenários) para o TSP
// ============================================================================

/// Gera 'n' pontos distribuídos uniformemente em um plano cartesiano
/// com coordenadas X e Y no intervalo [0.0, 100.0].
///
/// Utiliza std::mt19937 como gerador pseudo-aleatório. Se seed == 0,
/// utiliza std::random_device para obter uma seed não-determinística;
/// caso contrário, utiliza a seed fornecida (reprodutibilidade).
///
/// @param n    Número de pontos a gerar (deve ser > 0).
/// @param seed Seed para o gerador (0 = aleatória).
/// @return Vetor com 'n' pontos aleatórios.
/// @throws std::invalid_argument se n <= 0.
std::vector<Point> generateRandomPoints(int n, unsigned seed = 0);

/// Gera 'n' pontos perfeitamente distribuídos ao longo da circunferência
/// de um círculo de raio 'radius' centrado em 'center'.
///
/// Cada ponto i está no ângulo θ = 2πi/N, de modo que a rota sequencial
/// [0, 1, ..., N-1] corresponde ao perímetro do polígono regular inscrito,
/// cuja distância ótima é 2·N·radius·sin(π/N).
///
/// @param n      Número de pontos a gerar (deve ser > 0).
/// @param radius Raio do círculo (deve ser > 0).
/// @param center Centro do círculo (padrão: {50.0, 50.0}).
/// @return Vetor com 'n' pontos na circunferência.
/// @throws std::invalid_argument se n <= 0 ou radius <= 0.
std::vector<Point> generateCircularPoints(int n, double radius,
                                          Point center = {50.0, 50.0});

#endif // GENERATORS_HPP
