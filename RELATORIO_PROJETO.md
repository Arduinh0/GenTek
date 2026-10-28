# 📄 Relatório Técnico: Algoritmo Genético Aplicado ao Problema do Caixeiro Viajante (TSP 2D)

**Disciplina:** Aprendizado de Máquina  
**Curso:** Ciência da Computação / Engenharia  
**Data:** 28 de Outubro de 2026  
**Linguagens & Tecnologias:** C++17 (MSVC / CMake), Python 3 (Pandas, Matplotlib)  

---

## 1. Introdução e Formulação Teórica

### 1.1. O Problema do Caixeiro Viajante (TSP)
O **Problema do Caixeiro Viajante (Traveling Salesperson Problem — TSP)** é um dos problemas clássicos mais estudados na ciência da computação e na otimização combinatória. Dado um conjunto de $N$ cidades no plano bidimensional com coordenadas $(x_i, y_i)$, o objetivo é determinar o circuito hamiltoniano de menor comprimento possível que visite cada cidade exatamente uma vez e retorne à cidade de partida.

O espaço de busca para $N$ cidades é fatorial:
$$\Omega = \frac{(N - 1)!}{2}$$
Para $N = 30$, o número de rotas possíveis é de aproximadamente $4.42 \times 10^{30}$. Para $N = 100$, o espaço atinge $\approx 4.67 \times 10^{155}$ soluções possíveis, tornando métodos de busca exaustiva (força bruta) computacionalmente inviáveis.

### 1.2. Modelagem Matemática
Seja $\pi = (\pi_0, \pi_1, \dots, \pi_{N-1})$ uma permutação dos índices das cidades $\{0, 1, \dots, N-1\}$. A distância euclidiana entre duas cidades $i$ e $j$ é:
$$d(i, j) = \sqrt{(x_i - x_j)^2 + (y_i - y_j)^2}$$

A função objetivo a ser **minimizada** é a distância cíclica total $D(\pi)$:
$$D(\pi) = \sum_{i=0}^{N-2} d(\pi_i, \pi_{i+1}) + d(\pi_{N-1}, \pi_0)$$

A função de aptidão (*fitness*) a ser **maximizada** pelo Algoritmo Genético é definida de forma inversamente proporcional à distância total da rota:
$$f(\pi) = \begin{cases} 
\frac{1}{D(\pi)}, & \text{se } D(\pi) > 0 \\ 
\infty \text{ (DBL\_MAX)}, & \text{se } D(\pi) = 0 
\end{cases}$$

---

## 2. Decisões de Arquitetura e Engenharia de Software (C++17)

A implementação foi desenvolvida seguindo os princípios de código limpo, modularidade e máxima eficiência de execução.

```
Trabalho 2/
├── include/
│   ├── types.hpp          # Tipos base (Point, Individual, DistMatrix)
│   ├── distance.hpp       # Pré-cálculo e avaliação cíclica de rotas
│   ├── fitness.hpp        # Função de aptidão com tratamento de borda
│   ├── generators.hpp     # Geradores de pontos aleatórios e circulares
│   └── genetic.hpp        # Configuração, telemetria e operadores genéticos
├── src/
│   ├── distance.cpp
│   ├── fitness.cpp
│   ├── generators.cpp
│   └── genetic.cpp
├── resultados/            # Logs CSV de telemetria gerados automaticamente
├── graficos/              # Gráficos em alta resolução (300 DPI)
├── CMakeLists.txt         # Configuração de compilação Release com otimização /O2
├── gerar_graficos.py      # Pipeline autônoma em Python (Pandas e Matplotlib)
└── main.cpp               # Ponto de entrada com menu interativo e pipeline 100% autônoma
```

### 2.1. Otimizações de Alto Impacto

1. **Matriz de Distâncias Pré-calculada ($O(1)$ na Avaliação):**
   - As distâncias entre todas as cidades são computadas uma única vez antes do início da evolução, armazenadas em uma matriz simétrica $N \times N$.
   - A simetria ($d_{ij} = d_{ji}$) reduz o custo de pré-computação pela metade e elimina completamente o cálculo de raízes quadradas (`std::sqrt`) durante as gerações.

2. **Estrutura `EvaluatedIndividual` com Cache de Métricas:**
   ```cpp
   struct EvaluatedIndividual {
       Individual route;     // std::vector<int> (permutação)
       double     distance;  // Cache da distância euclidiana total
       double     fitValue;  // Cache do valor de aptidão (1.0 / distance)
   };
   ```
   - **Impacto:** Em vez de recalcular a distância cíclica $O(N)$ a cada comparação do `std::sort` ($O(P \log P)$ comparações) ou nas competições do torneio de seleção, as comparações operam em tempo constante $O(1)$.
   - A complexidade da ordenação por época cai de $O(N \cdot P \log P)$ para $O(P \log P)$.

3. **Passagem por Referência Constante (`const &`):**
   - Evita cópias desnecessárias de vetores de indivíduos e da matriz de distâncias durante a execução dos operadores genéticos.

4. **Autonomia de Diretório (`findProjectRoot`):**
   - O executável C++ detecta dinamicamente a raiz do projeto antes de inicializar caminhos, garantindo que os arquivos CSV em `resultados/` e os gráficos em `graficos/` sejam sempre gravados na raiz, mesmo quando o binário é chamado a partir da subpasta `build/`.

5. **Otimização de Compilação no CMake:**
   - Compilação configurada com `/O2 /Oi /Ot /permissive-` (MSVC) e `-O3 -march=native` (GCC/Clang), reduzindo o tempo de execução por época para frações de milissegundo.

---

## 3. Operadores Genéticos Implementados

### 3.1. Representação do Cromossomo
- **Representação por Permutação:** Um indivíduo é codificado como um vetor `std::vector<int>` contendo exatamente os índices $\{0, 1, \dots, N-1\}$. Não há repetição de cidades no genoma; o retorno cíclico da última cidade para a primeira é tratado implicitamente na função de avaliação.

### 3.2. Inicialização da População
- Gerada através de permutações aleatórias completas usando `std::shuffle` com a biblioteca `<random>` (`std::mt19937`), garantindo distribuição uniforme no espaço amostral inicial.

### 3.3. Seleção por Torneio ($k = 3$)
- Em cada seleção, 3 indivíduos são sorteados aleatoriamente da população. O candidato com menor distância (maior aptidão) vence o torneio.
- **Vantagem:** Apresenta excelente equilíbrio entre pressão seletiva e manutenção da diversidade, evitando que indivíduos dominantes prematuros saturem a população.

### 3.4. Cruzamento: *Order Crossover* (OX)
O operador OX é especificamente projetado para problemas de permutação, pois preserva adjacências relativas sem gerar genes duplicados ou faltantes:
1. Sorteiam-se dois pontos de corte aleatórios $c_1 < c_2$.
2. O segmento $[c_1, c_2]$ do Progenitor 1 é copiado diretamente para a mesma posição no Filho 1.
3. As posições restantes são preenchidas com as cidades do Progenitor 2 na ordem em que aparecem (em sequência circular a partir de $c_2 + 1$), ignorando as cidades que já foram copiadas no passo 2.
4. O Filho 2 é gerado de maneira simétrica (segmento do Progenitor 2 e preenchimento pelo Progenitor 1).

### 3.5. Mutação por Inversão (*Inversion Mutation*)
- Com probabilidade $P_m$, sorteiam-se dois pontos de corte $i < j$ e inverte-se a subsequência entre eles via `std::reverse`.
- **Relevância para o TSP:** É teoricamente e empiricamente o operador de mutação mais eficaz para o TSP 2D, pois simula o movimento clássico de *2-Opt*, desfazendo cruzamentos de arestas no plano cartesiano.

### 3.6. Elitismo e Critérios de Paragem
- **Elitismo:** Os melhores indivíduos de cada geração (2 a 3 indivíduos) são transferidos intactos para a próxima geração, garantindo que a melhor solução encontrada nunca seja perdida por deriva genética.
- **Critérios de Paragem Duplos:**
  1. *Número Máximo de Épocas:* Teto global de gerações ($1500 \sim 4000$).
  2. *Estagnação Consecutiva:* Interrupção antecipada caso o melhor valor de aptidão não melhore após $X$ gerações consecutivas ($250 \sim 700$ épocas).

---

## 4. Metodologia Experimental e Cenários de Teste

Foram estabelecidos três cenários com objetivos complementares:

### Cenário 1: Aleatório ($N = 30$ Cidades)
- Cidades distribuídas uniformemente no plano cartesiano $[0.0, 100.0] \times [0.0, 100.0]$ com semente determinística (`seed = 42`).
- **Objetivo:** Avaliar a capacidade do algoritmo de descruzar arestas caóticas e otimizar trajetórias arbitrárias.

### Cenário 2: Circular Benchmark ($N = 30$ Cidades, Raio = 40.0)
- Cidades perfeitamente distribuídas sobre a circunferência de um círculo:
  $$x_i = 50 + R \cos\left(\frac{2\pi i}{N}\right), \quad y_i = 50 + R \sin\left(\frac{2\pi i}{N}\right)$$
- **Propriedade Fundamental:** A rota ótima global conhecida a priori é o perímetro do polígono regular inscrito de $N$ lados:
  $$\text{Perímetro Ótimo} = 2 \cdot N \cdot R \cdot \sin\left(\frac{\pi}{N}\right)$$
  Para $N = 30$ e $R = 40.0$:
  $$\text{Perímetro Ótimo} = 2 \cdot 30 \cdot 40 \cdot \sin\left(\frac{\pi}{30}\right) \approx \mathbf{250.8683}$$
- **Objetivo:** Validação matemática formal de convergência exata do algoritmo.

### Cenário 3: Alta Carga / Bónus ($N = 100$ Cidades, Raio = 100.0)
- Cenário circular de alta volumetria com espaço de busca astronômico ($100! \approx 9.33 \times 10^{157}$).
- Para $N = 100$ e $R = 100.0$:
  $$\text{Perímetro Ótimo} = 2 \cdot 100 \cdot 100 \cdot \sin\left(\frac{\pi}{100}\right) \approx \mathbf{628.2152}$$
- **Objetivo:** Testar a escalabilidade computacional, telemetria de tempo via `std::chrono` e robustez do elitismo/mutação em espaços de altíssima dimensionalidade.

---

## 5. Resultados Experimentais Consolidados

### 5.1. Tabela Comparativa de Resultados

| Parâmetro / Métrica | Cenário 1 (Aleatório) | Cenário 2 (Circular Padrão) | Cenário 3 (Circular Bónus) |
|---|---|---|---|
| **Número de Cidades ($N$)** | 30 | 30 | 100 |
| **Raio da Circunferência ($R$)** | N/A | 40.0 | 100.0 |
| **Tamanho da População ($P$)** | 300 | 300 | 500 |
| **Taxa de Cruzamento ($P_c$)** | 85.0% (OX) | 85.0% (OX) | 85.0% (OX) |
| **Taxa de Mutação ($P_m$)** | 5.0% (Inversão) | 5.0% (Inversão) | 6.0% (Inversão) |
| **Elitismo** | 2 indivíduos | 2 indivíduos | 3 indivíduos |
| **Limite de Estagnação** | 400 épocas | 400 épocas | 700 épocas |
| **Épocas até Paragem** | 633 | 498 | 1592 |
| **Razão de Paragem** | Estagnação | Estagnação | Estagnação |
| **Tempo Total de Execução** | **55.22 ms** (0.055 s) | **41.57 ms** (0.041 s) | **470.76 ms** (0.471 s) |
| **Tempo Médio por Época** | 0.087 ms/época | 0.083 ms/época | 0.296 ms/época |
| **Distância Inicial (Melhor)** | 1273.6156 | 1218.1344 | 10927.4491 |
| **Distância Final Obtida (AG)** | **529.5461** | **250.8683** | **628.2152** |
| **Distância Ótima Teórica** | N/A | **250.8683** | **628.2152** |
| **Diferença Absoluta** | N/A | **0.0000** | **0.0000** |
| **Erro Relativo (%)** | **Melhoria: 58.42%** | **0.0000% (100% Exato)** | **0.0000% (100% Exato)** |

---

## 6. Análise e Discussão dos Resultados

### 6.1. Convergência Global Exata nos Cenários Circulares
O resultado mais expressivo do projeto foi a obtenção de **0.0000% de erro relativo** tanto no cenário circular com $N = 30$ quanto no de alta carga com $N = 100$.
- O algoritmo identificou com precisão cirúrgica a sequência circular sem cruzamentos (`46 -> 45 -> 44 -> ... -> 47 -> 46`), atingindo o valor exato de **628.2152**.
- A combinação de **Order Crossover (OX)** com **Mutação por Inversão** provou ser sinérgica: o OX manteve blocos de vizinhança intactos enquanto a inversão desfez os entrelaçamentos que cruzavam o interior do círculo.

### 6.2. Dinâmica das Curvas de Convergência
Ao analisar a evolução temporal da população nos gráficos de telemetria:
1. **Fase Inicial (Épocas 0 a 150):** Observa-se uma queda exponencial na distância média e na melhor distância. A diversidade aleatória inicial permite saltos rápidos em direção a bacias de atração favoráveis.
2. **Fase Intermediária (Épocas 150 a 600):** A distância média converge gradualmente em direção à melhor distância, indicando homogeneização genética controlada e refinamento local das arestas.
3. **Fase de Refinamento e Estagnação:** O elitismo preserva a melhor solução enquanto mutações por inversão raras realizam microajustes até que o critério de estagnação finalize o processo, poupando ciclos de CPU.

### 6.3. Desempenho e Eficiência Computacional
- O tempo total de execução para as 3 experiências combinadas foi de **menos de 0.6 segundo**.
- O cenário bônus de 100 cidades processou quase 1600 épocas (com população de 500 indivíduos = 800.000 avaliações de indivíduos) em apenas **470 ms**.
- Esse resultado comprova que o **cache de avaliação** em `EvaluatedIndividual` somado ao pré-cálculo da matriz de distâncias elimina os gargalos que historicamente tornavam Algoritmos Genéticos em linguagens interpretadas lentos para grafos médios e grandes.

---

## 7. Pipeline Autônoma e Geração Gráfica

O projeto conta com um ciclo totalmente autônomo entre o motor C++ e a camada visual em Python:
1. O usuário executa `tsp_ga` (interativamente ou via script).
2. O C++ gera a telemetria em formato CSV dentro da pasta `resultados/`:
   - `log_aleatorio.csv`, `log_circular.csv`, `log_bonus_100.csv`
   - `cidades_aleatorio.csv`, `cidades_circular.csv`, `cidades_bonus_100.csv`
3. O C++ invoca automaticamente `python gerar_graficos.py` ao concluir.
4. O script Python analisa dinamicamente as dimensões $N$ e $R$ dos dados e exporta as figuras em 300 DPI para a pasta `graficos/`:
   - **Curvas de Convergência:** `convergencia_aleatorio.png`, `convergencia_circular.png`, `convergencia_bonus_100.png`.
   - **Painel Comparativo:** `convergencia_comparativa.png` (painel integrado lado a lado).
   - **Traçados dos Percursos 2D:** `rota_aleatorio.png`, `rota_circular_30.png`, `rota_bonus_100.png`.

---

## 8. Conclusão

O projeto atingiu com louvor todos os objetivos acadêmicos e técnicos propostos:
- **Corretude Algorítmica:** Comprovação empírica e analítica da capacidade de convergência ao ótimo global.
- **Rigor de Engenharia de Software:** Arquitetura desacoplada em C++17, ausência de vazamento de memória, uso de referências constantes e tratamento seguro do sistema de arquivos.
- **Escalabilidade:** Execução em frações de segundo para problemas com $N=100$ e espaço amostral de $\approx 10^{157}$ rotas.
- **Automação Completa:** Integração transparente entre C++ e Python com geração automatizada de relatórios visuais prontos para publicação.
