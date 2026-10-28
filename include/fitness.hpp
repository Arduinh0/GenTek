#ifndef FITNESS_HPP
#define FITNESS_HPP

#include "types.hpp"

// ============================================================================
// fitness.hpp — Função de Aptidão (Fitness)
// ============================================================================

/// Calcula a aptidão de um indivíduo como o inverso da distância total da rota.
/// Quanto menor a distância, maior o fitness (objetivo: maximizar fitness).
///
/// Caso especial: se a distância for 0 (rota degenerada com 0 ou 1 cidade),
/// retorna std::numeric_limits<double>::max() para evitar divisão por zero.
///
/// @param route  Permutação de índices das cidades.
/// @param dist   Matriz de distâncias pré-calculada.
/// @return Valor de aptidão = 1.0 / distância_total.
double fitness(const Individual& route, const DistMatrix& dist);

#endif // FITNESS_HPP
