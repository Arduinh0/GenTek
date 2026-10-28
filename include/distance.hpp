#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include "types.hpp"
#include <vector>

// ============================================================================
// distance.hpp — Pré-cálculo de Distâncias e Avaliação de Rotas
// ============================================================================

/// Calcula a distância euclidiana entre dois pontos.
/// @param a Primeiro ponto.
/// @param b Segundo ponto.
/// @return Distância euclidiana sqrt((ax-bx)² + (ay-by)²).
double euclideanDistance(const Point& a, const Point& b);

/// Pré-calcula a matriz simétrica NxN de distâncias euclidianas entre todas
/// as cidades. Explora a simetria: calcula apenas o triângulo superior e
/// espelha para o inferior. A diagonal é zero.
/// @param cities Vetor de pontos representando as cidades.
/// @return Matriz NxN de distâncias.
DistMatrix computeDistanceMatrix(const std::vector<Point>& cities);

/// Calcula a distância total de uma rota (ciclo hamiltoniano).
/// Percorre a rota na ordem dada e soma a distância de retorno à cidade
/// de origem (route[N-1] → route[0]).
/// @param route  Permutação de índices das cidades.
/// @param dist   Matriz de distâncias pré-calculada.
/// @return Distância total da rota cíclica.
double routeDistance(const Individual& route, const DistMatrix& dist);

#endif // DISTANCE_HPP
