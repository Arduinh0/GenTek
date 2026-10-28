#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gerar_graficos.py — Fase 5: Visualização dos Dados e Relatórios Gráficos (TSP GA)

Gera:
1. Curvas de Convergência (Melhor Distância e Distância Média vs Época)
2. Traçados 2D dos Percursos Encontrados (Cenário Aleatório, Circular e Bónus N=100)
3. Painéis Comparativos Completos (300 DPI)

Saída: pasta graficos/
"""

import os
import math
import pandas as pd
import matplotlib.pyplot as plt

# Configuração global de estilo para artigos e relatórios acadêmicos
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 13
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 10
plt.rcParams['figure.titlesize'] = 14

OUTPUT_DIR = "graficos"
RESULTS_DIR = "resultados"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def load_cities_from_csv(csv_path):
    """Carrega as coordenadas X, Y geradas pelo C++ se disponíveis."""
    if os.path.exists(csv_path):
        df = pd.read_csv(csv_path)
        return {int(row['Cidade']): (float(row['X']), float(row['Y'])) for _, row in df.iterrows()}
    return None

def compute_circular_coords(n_cities, radius, center=(50.0, 50.0)):
    """Gera analiticamente as coordenadas do círculo conforme implementado em C++."""
    coords = {}
    angle_step = 2.0 * math.pi / n_cities
    for i in range(n_cities):
        theta = angle_step * i
        x = center[0] + radius * math.cos(theta)
        y = center[1] + radius * math.sin(theta)
        coords[i] = (x, y)
    return coords

# -----------------------------------------------------------------------------
# 1. Plot de Curvas de Convergência Individuais
# -----------------------------------------------------------------------------
def plot_convergence(log_file, title, output_name, theoretical_opt=None):
    csv_path = os.path.join(RESULTS_DIR, log_file)
    if not os.path.exists(csv_path):
        print(f"[AVISO] Ficheiro nao encontrado: {csv_path}")
        return

    df = pd.read_csv(csv_path)
    
    fig, ax = plt.subplots(figsize=(9, 5.5), dpi=300)

    # Curva da distância média
    ax.plot(df['Epoca'], df['DistanciaMedia'], label='Distância Média da População',
            color='#E67E22', linestyle='--', linewidth=1.8, alpha=0.85)

    # Curva da melhor distância
    ax.plot(df['Epoca'], df['MelhorDistancia'], label='Melhor Distância (Elitismo)',
            color='#1F77B4', linewidth=2.2)

    # Linha do ótimo teórico (se houver)
    if theoretical_opt is not None:
        ax.axhline(y=theoretical_opt, color='#2CA02C', linestyle=':', linewidth=2.0,
                   label=f'Ótimo Teórico ({theoretical_opt:.2f})')

    # Marcadores inicial e final
    init_best = df['MelhorDistancia'].iloc[0]
    final_best = df['MelhorDistancia'].iloc[-1]
    final_epoch = df['Epoca'].iloc[-1]

    ax.scatter([0], [init_best], color='#D62728', zorder=5, s=60, label=f'Início: {init_best:.1f}')
    ax.scatter([final_epoch], [final_best], color='#1F77B4', zorder=5, s=70,
               edgecolors='black', label=f'Final: {final_best:.2f} (Época {final_epoch})')

    ax.set_title(title, fontweight='bold', pad=12)
    ax.set_xlabel('Época (Geração)')
    ax.set_ylabel('Distância Euclidiana Total')
    ax.grid(True, linestyle='--', alpha=0.45)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9, loc='upper right')

    out_path = os.path.join(OUTPUT_DIR, output_name)
    plt.tight_layout()
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"  [OK] Grafico salvo: {out_path}")

# -----------------------------------------------------------------------------
# 2. Plot dos Traçados 2D dos Percursos (Rotas)
# -----------------------------------------------------------------------------
def plot_route(log_file, coords, title, output_name, radius=None, center=(50.0, 50.0)):
    csv_path = os.path.join(RESULTS_DIR, log_file)
    if not os.path.exists(csv_path):
        return

    df = pd.read_csv(csv_path)
    last_row = df.iloc[-1]
    route_str = str(last_row['MelhorRota'])
    final_dist = float(last_row['MelhorDistancia'])
    
    route_indices = [int(x) for x in route_str.split('-')]
    # Fechar ciclo hamiltoniano
    route_indices.append(route_indices[0])

    xs = [coords[idx][0] for idx in route_indices]
    ys = [coords[idx][1] for idx in route_indices]

    fig, ax = plt.subplots(figsize=(7.5, 7.5), dpi=300)

    # Circunferência teórica de referência (se for cenário circular)
    if radius is not None:
        circle = plt.Circle(center, radius, color='#BDC3C7', fill=False,
                            linestyle=':', linewidth=1.5, label=f'Circunferência (R={radius})')
        ax.add_patch(circle)

    # Traçado das arestas do percurso
    ax.plot(xs, ys, color='#2980B9', linewidth=1.8, alpha=0.9, zorder=2,
            label=f'Percurso AG (Distância: {final_dist:.2f})')

    # Marcadores de cidades
    city_xs = [coords[i][0] for i in range(len(coords))]
    city_ys = [coords[i][1] for i in range(len(coords))]
    ax.scatter(city_xs, city_ys, color='#C0392B', s=45, zorder=3, edgecolors='white',
               linewidth=0.8, label=f'Cidades (N={len(coords)})')

    # Destaque da cidade de origem
    ax.scatter([xs[0]], [ys[0]], color='#27AE60', s=130, marker='*', zorder=4,
               edgecolors='black', label=f'Origem (Cidade {route_indices[0]})')

    ax.set_title(title, fontweight='bold', pad=12)
    ax.set_xlabel('Coordenada X')
    ax.set_ylabel('Coordenada Y')
    ax.set_aspect('equal', 'box')
    ax.grid(True, linestyle='--', alpha=0.4)
    ax.legend(frameon=True, facecolor='white', framealpha=0.95, loc='upper right')

    out_path = os.path.join(OUTPUT_DIR, output_name)
    plt.tight_layout()
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"  [OK] Grafico de rota salvo: {out_path}")

# -----------------------------------------------------------------------------
# 3. Painel Comparativo de Convergência (3 Subplots)
# -----------------------------------------------------------------------------
def plot_comparative_convergence():
    logs = [
        ("log_aleatorio.csv", "Cenário Aleatório (N = 30)", None),
        ("log_circular.csv", "Cenário Circular Benchmark (N = 30, R = 40)", 250.8683),
        ("log_bonus_100.csv", "Cenário Bónus Alta Carga (N = 100, R = 100)", 628.2152)
    ]

    fig, axes = plt.subplots(1, 3, figsize=(18, 5.2), dpi=300)

    for i, (log_file, title, opt) in enumerate(logs):
        csv_path = os.path.join(RESULTS_DIR, log_file)
        if not os.path.exists(csv_path):
            continue
        df = pd.read_csv(csv_path)
        ax = axes[i]

        ax.plot(df['Epoca'], df['DistanciaMedia'], label='Média População',
                color='#E67E22', linestyle='--', linewidth=1.5, alpha=0.8)
        ax.plot(df['Epoca'], df['MelhorDistancia'], label='Melhor Distância',
                color='#1F77B4', linewidth=2.0)

        if opt is not None:
            ax.axhline(y=opt, color='#2CA02C', linestyle=':', linewidth=1.8,
                       label=f'Ótimo ({opt:.1f})')

        ax.set_title(title, fontweight='bold', fontsize=11)
        ax.set_xlabel('Épocas')
        ax.set_ylabel('Distância')
        ax.grid(True, linestyle='--', alpha=0.4)
        ax.legend(fontsize=9, loc='upper right')

    fig.suptitle('Comparativo da Dinâmica Evolutiva do Algoritmo Genético', fontsize=14, fontweight='bold')
    out_path = os.path.join(OUTPUT_DIR, "convergencia_comparativa.png")
    plt.tight_layout()
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"  [OK] Painel comparativo salvo: {out_path}")

# -----------------------------------------------------------------------------
# 4. Pipeline Principal
# -----------------------------------------------------------------------------
def main():
    print("=" * 65)
    print("GERADOR DE GRAFICOS TSP - ALGORITMO GENETICO (FASE 5)")
    print("=" * 65)

    # 1. Curvas de Convergência
    print("\n--- Gerando Curvas de Convergencia ---")
    plot_convergence("log_aleatorio.csv",
                     "Convergência do AG: Cenário Aleatório (N = 30)",
                     "convergencia_aleatorio.png")

    plot_convergence("log_circular.csv",
                     "Convergência do AG: Cenário Circular Benchmark (N = 30, R = 40)",
                     "convergencia_circular.png",
                     theoretical_opt=250.8683)

    plot_convergence("log_bonus_100.csv",
                     "Convergência do AG: Cenário Bónus Alta Carga (N = 100, R = 100)",
                     "convergencia_bonus_100.png",
                     theoretical_opt=628.2152)

    plot_comparative_convergence()

    # 2. Rotas 2D
    print("\n--- Gerando Visualizacoes 2D das Rotas ---")
    # Rota Aleatória
    coords_rand = load_cities_from_csv(os.path.join(RESULTS_DIR, "cidades_aleatorio.csv"))
    if coords_rand:
        plot_route("log_aleatorio.csv", coords_rand,
                   "Melhor Percurso Encontrado: Cenário Aleatório (N = 30)",
                   "rota_aleatorio.png")

    # Rota Circular (N = 30, R = 40)
    coords_circ = load_cities_from_csv(os.path.join(RESULTS_DIR, "cidades_circular.csv"))
    if not coords_circ:
        coords_circ = compute_circular_coords(30, 40.0)
    plot_route("log_circular.csv", coords_circ,
               "Melhor Percurso Encontrado: Cenário Circular (N = 30, R = 40)",
               "rota_circular_30.png", radius=40.0)

    # Rota Bónus (N = 100, R = 100)
    coords_bonus = load_cities_from_csv(os.path.join(RESULTS_DIR, "cidades_bonus_100.csv"))
    if not coords_bonus:
        coords_bonus = compute_circular_coords(100, 100.0)
    plot_route("log_bonus_100.csv", coords_bonus,
               "Melhor Percurso Encontrado: Cenário Bónus Alta Carga (N = 100, R = 100)",
               "rota_bonus_100.png", radius=100.0)

    print("\n" + "=" * 65)
    print("TODOS OS GRAFICOS GERADOS COM SUCESSO EM:", os.path.abspath(OUTPUT_DIR))
    print("=" * 65)

if __name__ == '__main__':
    main()
