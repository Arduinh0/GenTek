#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>

// ============================================================================
// types.hpp — Modelagem de Dados Base para o AG-TSP
// ============================================================================

/// Representa um ponto (cidade) no plano cartesiano 2D.
struct Point {
    double x;
    double y;
};

/// Um indivíduo é uma permutação de índices das cidades (genes).
/// Ex: {2, 0, 3, 1} significa visitar as cidades na ordem 2 → 0 → 3 → 1 → 2.
using Individual = std::vector<int>;

/// Matriz de distâncias NxN pré-calculada.
/// dist_matrix[i][j] = distância euclidiana entre a cidade i e a cidade j.
using DistMatrix = std::vector<std::vector<double>>;

#endif // TYPES_HPP
