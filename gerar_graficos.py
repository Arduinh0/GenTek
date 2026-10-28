#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gerar_graficos.py — Visualização dos Dados e Relatórios Gráficos (TSP GA)

Gera:
1. Curvas de Convergência Dinâmicas (Melhor Distância e Média vs Época)
2. Traçados 2D dos Percursos Encontrados com N dinâmico
3. Painel Comparativo Integrado (300 DPI)

Saída: pasta graficos/ na raiz do projeto
"""

import os
import math
import pandas as pd
import matplotlib.pyplot as plt

# Configuração global de estilo
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 13
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['xtick.labelsize'] = 10
plt.rcParams['ytick.labelsize'] = 10
plt.rcParams['legend.fontsize'] = 10
plt.rcParams['figure.titlesize'] = 14

# Localização robusta em relação ao diretório deste script (raiz do projeto)
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
RESULTS_DIR = os.path.join(BASE_DIR, "resultados")
OUTPUT_DIR = os.path.join(BASE_DIR, "graficos")
os.makedirs(OUTPUT_DIR, exist_ok=True)

def load_cities_from_csv(csv_path):
    """Carrega as coordenadas X, Y geradas pelo C++ se disponíveis."""
    if os.path.exists(csv_path):
        df = pd.read_csv(csv_path)
        return {int(row['Cidade']): (float(row['X']), float(row['Y'])) for _, row in df.iterrows()}
    return None

def get_n_cities_from_log(csv_path):
    """Lê dinamicamente o número de cidades contando os nós da rota final no CSV."""
    if os.path.exists(csv_path):
        df = pd.read_csv(csv_path)
        if not df.empty and 'MelhorRota' in df.columns:
            last_route = str(df['MelhorRota'].iloc[-1])
            return len(last_route.split('-'))
    return 30

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

def extract_radius_from_coords(coords, center=(50.0, 50.0)):
    """Extrai o raio utilizado a partir das coordenadas dos pontos."""
    if coords and len(coords) > 0:
        x0, y0 = coords[0]
        return math.hypot(x0 - center[0], y0 - center[1])
    return 40.0

# -----------------------------------------------------------------------------
# 1. Curvas de Convergência Individuais com Títulos Dinâmicos
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

    # Linha do ótimo teórico
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
# 2. Plot dos Traçados 2D dos Percursos com N e R Dinâmicos
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
    n_real = len(route_indices)

    # Fechar ciclo hamiltoniano
    route_indices.append(route_indices[0])

    xs = [coords[idx][0] for idx in route_indices]
    ys = [coords[idx][1] for idx in route_indices]

    fig, ax = plt.subplots(figsize=(7.5, 7.5), dpi=300)

    # Circunferência teórica de referência
    if radius is not None:
        circle = plt.Circle(center, radius, color='#BDC3C7', fill=False,
                            linestyle=':', linewidth=1.5, label=f'Circunferência (R={radius:.1f})')
        ax.add_patch(circle)

    # Traçado das arestas do percurso
    ax.plot(xs, ys, color='#2980B9', linewidth=1.8, alpha=0.9, zorder=2,
            label=f'Percurso AG (Distância: {final_dist:.2f})')

    # Marcadores de cidades
    city_xs = [coords[i][0] for i in range(len(coords))]
    city_ys = [coords[i][1] for i in range(len(coords))]
    ax.scatter(city_xs, city_ys, color='#C0392B', s=45, zorder=3, edgecolors='white',
               linewidth=0.8, label=f'Cidades (N = {n_real})')

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
# 3. Painel Comparativo Integrado com N e R Dinâmicos
# -----------------------------------------------------------------------------
def plot_comparative_convergence(info_list):
    fig, axes = plt.subplots(1, len(info_list), figsize=(6.0 * len(info_list), 5.2), dpi=300)

    for i, (log_file, title, opt) in enumerate(info_list):
        csv_path = os.path.join(RESULTS_DIR, log_file)
        if not os.path.exists(csv_path):
            continue
        df = pd.read_csv(csv_path)
        ax = axes[i] if len(info_list) > 1 else axes

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
    print("GERADOR DE GRAFICOS TSP - ALGORITMO GENETICO (FASE 5 DINAMICA)")
    print("=" * 65)

    # 1. Carregar coordenadas e determinar N e R dinamicamente
    # Cenário 1: Aleatório
    coords_rand = load_cities_from_csv(os.path.join(RESULTS_DIR, "cidades_aleatorio.csv"))
    n_rand = len(coords_rand) if coords_rand else get_n_cities_from_log(os.path.join(RESULTS_DIR, "log_aleatorio.csv"))

    # Cenário 2: Circular Padrão
    coords_circ = load_cities_from_csv(os.path.join(RESULTS_DIR, "cidades_circular.csv"))
    n_circ = len(coords_circ) if coords_circ else get_n_cities_from_log(os.path.join(RESULTS_DIR, "log_circular.csv"))
    r_circ = extract_radius_from_coords(coords_circ) if coords_circ else 40.0
    if not coords_circ:
        coords_circ = compute_circular_coords(n_circ, r_circ)
    opt_circ = 2.0 * n_circ * r_circ * math.sin(math.pi / n_circ)

    # Cenário 3: Circular Bónus
    coords_bonus = load_cities_from_csv(os.path.join(RESULTS_DIR, "cidades_bonus_100.csv"))
    n_bonus = len(coords_bonus) if coords_bonus else get_n_cities_from_log(os.path.join(RESULTS_DIR, "log_bonus_100.csv"))
    r_bonus = extract_radius_from_coords(coords_bonus) if coords_bonus else 100.0
    if not coords_bonus:
        coords_bonus = compute_circular_coords(n_bonus, r_bonus)
    opt_bonus = 2.0 * n_bonus * r_bonus * math.sin(math.pi / n_bonus)

    print(f"Dimensoes detectadas nos CSVs:")
    print(f"  - Cenario Aleatorio:      N = {n_rand}")
    print(f"  - Cenario Circular:       N = {n_circ}, Raio = {r_circ:.1f} (Otimo: {opt_circ:.2f})")
    print(f"  - Cenario Bonus:          N = {n_bonus}, Raio = {r_bonus:.1f} (Otimo: {opt_bonus:.2f})\n")

    # 2. Curvas de Convergência com Títulos Dinâmicos
    print("--- Gerando Curvas de Convergencia Dinamicas ---")
    plot_convergence("log_aleatorio.csv",
                     f"Convergência do AG: Cenário Aleatório (N = {n_rand})",
                     "convergencia_aleatorio.png")

    plot_convergence("log_circular.csv",
                     f"Convergência do AG: Cenário Circular Benchmark (N = {n_circ}, R = {r_circ:.1f})",
                     "convergencia_circular.png",
                     theoretical_opt=opt_circ)

    plot_convergence("log_bonus_100.csv",
                     f"Convergência do AG: Cenário Bónus Alta Carga (N = {n_bonus}, R = {r_bonus:.1f})",
                     "convergencia_bonus_100.png",
                     theoretical_opt=opt_bonus)

    # Painel comparativo
    comparative_info = [
        ("log_aleatorio.csv", f"Cenário Aleatório (N = {n_rand})", None),
        ("log_circular.csv", f"Cenário Circular (N = {n_circ}, R = {r_circ:.1f})", opt_circ),
        ("log_bonus_100.csv", f"Cenário Bónus (N = {n_bonus}, R = {r_bonus:.1f})", opt_bonus)
    ]
    plot_comparative_convergence(comparative_info)

    # 3. Visualizações 2D das Rotas
    print("\n--- Gerando Visualizacoes 2D das Rotas ---")
    if coords_rand:
        plot_route("log_aleatorio.csv", coords_rand,
                   f"Melhor Percurso Encontrado: Cenário Aleatório (N = {n_rand})",
                   "rota_aleatorio.png")

    plot_route("log_circular.csv", coords_circ,
               f"Melhor Percurso Encontrado: Cenário Circular (N = {n_circ}, R = {r_circ:.1f})",
               "rota_circular_30.png", radius=r_circ)

    plot_route("log_bonus_100.csv", coords_bonus,
               f"Melhor Percurso Encontrado: Cenário Bónus Alta Carga (N = {n_bonus}, R = {r_bonus:.1f})",
               "rota_bonus_100.png", radius=r_bonus)

    print("\n" + "=" * 65)
    print("TODOS OS GRAFICOS GERADOS COM SUCESSO EM:", os.path.abspath(OUTPUT_DIR))
    print("=" * 65)

if __name__ == '__main__':
    main()
