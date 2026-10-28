# 🧬 GenTek — Algoritmo Genético para o Problema do Caixeiro Viajante (TSP 2D)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.10%2B-orange.svg?logo=cmake)](https://cmake.org/)
[![Python](https://img.shields.io/badge/Python-3.8%2B-green.svg?logo=python)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Implementação modular, robusta e de alta performance de um **Algoritmo Genético (AG)** aplicado ao **Problema do Caixeiro Viajante em 2D (Traveling Salesperson Problem - TSP)**. O projeto foi estruturado em **C++ moderno (C++17)** com compilação otimizada (`/O2` / `-O3`), acompanhado por uma pipeline em **Python** para análise de telemetria e geração de relatórios gráficos de alta resolução (300 DPI).

---

## 📌 Sumário
- [Visão Geral e Fases do Projeto](#-visão-geral-e-fases-do-projeto)
- [Arquitetura e Boas Práticas](#-arquitetura-e-boas-práticas)
- [Estrutura do Repositório](#-estrutura-do-repositório)
- [Requisitos e Dependências](#-requisitos-e-dependências)
- [Guia de Instalação e Execução](#-guia-de-instalação-e-execução)
- [Resultados Experimentais](#-resultados-experimentais)
- [Galeria de Gráficos](#-galeria-de-gráficos)

---

## 🚀 Visão Geral e Fases do Projeto

O projeto foi concebido e implementado de forma modular em 5 fases evolutivas:

1. **Fase 1 — Estrutura Base e Matemática:** Modelagem de tipos (`Point`, `Individual`, `DistMatrix`), pré-cálculo da matriz simétrica de distâncias euclidianas $O(1)$, cálculo de rota cíclica hamiltoniana, função de aptidão ($1/\text{distância}$) com proteção contra divisão por zero e geradores pseudo-aleatórios com `<random>` (cenários aleatório e circular benchmark).
2. **Fase 2 — Operadores Genéticos e Ciclo de Evolução:** Inicialização aleatória com `std::shuffle`, Seleção por Torneio ($k=3$), Cruzamento *Order Crossover* (OX), Mutação por Inversão com taxa parametrizável ($P_m$), Elitismo e critérios duplos de paragem (máximo de gerações e estagnação consecutiva).
3. **Fase 3 — Telemetria e Exportação de Dados:** Registro de telemetria época a época (`Epoca,MelhorDistancia,DistanciaMedia,MelhorRota`) com streaming direto para CSV e callbacks flexíveis.
4. **Fase 4 — Alta Volumetria e Telemetria de Tempo:** Cenário de alta carga ($N = 100$ cidades), medição de tempo de alta resolução via `std::chrono::high_resolution_clock`, organização automática de saídas via `<filesystem>` e flags de otimização máxima (`/O2` / `-O3`).
5. **Fase 5 — Menu Interativo e Pipeline Gráfica:** Interface interativa via `std::cin` com valores padrão de fallback, exportação de coordenadas e script Python (`gerar_graficos.py`) para geração automática de curvas de convergência e visualização 2D dos percursos a 300 DPI.

---

## ⚙️ Arquitetura e Boas Práticas

- **Cache de Avaliação (`EvaluatedIndividual`):** Cada indivíduo armazena sua distância e fitness calculados uma única vez na instanciação. Isto reduz a complexidade da ordenação da população de $O(N \cdot P \log P)$ para $O(P \log P)$, e das comparações de torneio de $O(N)$ para $O(1)$.
- **Matriz de Distâncias Simétrica:** Pré-computação das distâncias euclidianas $N \times N$, explorando simetria ($d_{ij} = d_{ji}$) e eliminando chamadas repetitivas de `std::sqrt` durante a evolução.
- **Passagem por Referência Constante (`const &`):** Eliminação de cópias de vetores e matrizes pesadas durante a seleção e cálculo de aptidão.
- **Sobrescrita Segura com `<filesystem>`:** Criação dinâmica da pasta `resultados/` e truncamento de arquivos via `std::ios::out | std::ios::trunc`.

---

## 📂 Estrutura do Repositório

```
GenTek/
├── CMakeLists.txt             # Configuração de build C++ (padrão C++17 e otimizações Release)
├── README.md                  # Documentação completa do projeto
├── main.cpp                   # Ponto de entrada interativo e orquestrador das experiências
├── gerar_graficos.py          # Pipeline Python (pandas + matplotlib) para geração de gráficos
├── include/                   # Headers da aplicação
│   ├── types.hpp              # Definição de Point, Individual e DistMatrix
│   ├── distance.hpp           # Pré-cálculo e avaliação cíclica de distâncias
│   ├── fitness.hpp            # Cálculo de aptidão com proteção de borda
│   ├── generators.hpp         # Gerador uniforme 2D e gerador circular determinístico
│   └── genetic.hpp            # GAConfig, GAResult, GenerationData e operadores genéticos
├── src/                       # Implementação dos módulos C++
│   ├── distance.cpp           # euclideanDistance, computeDistanceMatrix, routeDistance
│   ├── fitness.cpp            # fitness()
│   ├── generators.cpp         # generateRandomPoints(), generateCircularPoints()
│   └── genetic.cpp            # tournamentSelection, orderCrossover, inversionMutation, runGA
├── resultados/                # Diretório de telemetria (gerado automaticamente)
│   ├── log_aleatorio.csv      # Log época a época (Cenário Aleatório)
│   ├── log_circular.csv       # Log época a época (Cenário Circular N=30)
│   ├── log_bonus_100.csv      # Log época a época (Cenário Bónus N=100)
│   └── cidades_*.csv          # Coordenadas X, Y dos pontos para visualização
└── graficos/                  # Figuras e gráficos em alta resolução (300 DPI)
    ├── convergencia_*.png     # Curvas de evolução da população
    ├── convergencia_comparativa.png # Painel lado a lado das 3 experiências
    └── rota_*.png             # Traçados 2D das melhores soluções encontradas
```

---

## 📋 Requisitos e Dependências

### C++ (Motor de Execução)
- **Compilador compatível com C++17:**
  - MSVC 2019+ (Visual Studio 16.0 ou superior no Windows)
  - GCC 9+ (Linux)
  - Clang 10+ (macOS / Linux)
- **CMake:** Versão 3.10 ou superior.

### Python (Pipeline Gráfica)
- **Python:** Versão 3.8 ou superior.
- **Bibliotecas:**
  - `pandas`
  - `matplotlib`

---

## 🛠️ Guia de Instalação e Execução

### 1. Clonar o Repositório
```bash
git clone https://github.com/Arduinh0/GenTek.git
cd GenTek
```

### 2. Instalar Dependências do Python
```bash
pip install pandas matplotlib
```

### 3. Compilar a Aplicação C++

#### No Windows (Visual Studio / Developer Command Prompt):
```powershell
# Criação do diretório de build e compilação em Release
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

#### No Linux / macOS (GCC ou Clang):
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

### 4. Executar as Experiências

#### Modo Interativo:
Execute o binário gerado. O programa solicitará os parâmetros de cada cenário pelo console (ou assumirá os valores padrão recomendados ao pressionar Enter):
```powershell
.\build\tsp_ga.exe
```

#### Modo Automatizado (Pipeline):
Para executar com os valores de baseline automaticamente via redirecionamento de entrada:
```powershell
# Windows (PowerShell)
"30`n30 40.0`n100 100.0`n" | .\build\tsp_ga.exe

# Linux / macOS (Bash)
printf "30\n30 40.0\n100 100.0\n" | ./build/tsp_ga
```

### 5. Gerar os Gráficos de Telemetria
Após o término da execução do C++, processe os arquivos gerados na pasta `resultados/`:
```bash
python gerar_graficos.py
```
Todas as imagens serão exportadas automaticamente para a pasta `graficos/`.

---

## 📊 Resultados Experimentais

A tabela a seguir resume as métricas obtidas durante os testes formais de baseline:

| Experiência | Cidades ($N$) | Raio ($R$) | População | Gerações | Tempo Total | Distância Inicial | Distância Final | Ótimo Teórico | Erro Relativo |
|---|---|---|---|---|---|---|---|---|---|
| **1. Aleatório** | 30 | N/A | 300 | 633 | **55.19 ms** | 1273.62 | **529.55** | N/A | Melhoria: 58.42% |
| **2. Circular Padrão** | 30 | 40.0 | 300 | 498 | **43.23 ms** | 1218.13 | **250.8683** | **250.8683** | **0.0000%** (100% exato) |
| **3. Circular Bónus** | 100 | 100.0 | 500 | 1592 | **478.86 ms** | 10927.45 | **628.2152** | **628.2152** | **0.0000%** (100% exato) |

> **Nota Teórica:** O perímetro ótimo teórico do polígono regular de $N$ lados inscrito no círculo é dado por:
> $$\text{Perímetro Ótimo} = 2 \cdot N \cdot R \cdot \sin\left(\frac{\pi}{N}\right)$$
> Tanto no cenário de 30 cidades quanto no de 100 cidades, o Algoritmo Genético convergiu com precisão absoluta de **0.0000%** de erro em menos de 0.5 segundo, descruzando totalmente o grafo cíclico.

---

## 🖼️ Galeria de Gráficos

Os gráficos produzidos pelo `gerar_graficos.py` estão disponíveis na pasta [`graficos/`](graficos/):

- **Painel Comparativo de Convergência:** Visualização integrada da evolução da `DistanciaMedia` e `MelhorDistancia` nos três cenários lado a lado.
- **Traçados dos Percursos 2D:** Reconstrução gráfica das soluções encontradas no plano cartesiano e nas circunferências de raio 40 e 100.

---

## 📜 Licença

Este projeto foi desenvolvido para fins acadêmicos na disciplina de **Aprendizado de Máquina**.
Distribuído sob licença aberta para estudos e pesquisas.
