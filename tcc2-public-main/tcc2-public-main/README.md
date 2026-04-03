# tcc2-public

Projeto de TCC voltado para a otimização de tabelas esportivas com restrições de mando e deslocamento. O código atual trabalha com calendários de turno e returno para a Superliga de vôlei masculina e feminina, usando uma combinação de construção inicial por método do polígono, Simulated Annealing e Iterated Local Search.

O repositório concentra três coisas principais:

- a implementação em C++ do algoritmo de geração e melhoria de calendários;
- instâncias baseadas em dados oficiais e tabelas oficiais de temporadas reais;
- um script auxiliar em Python para converter tabelas com nomes de times para a codificação numérica usada pelo programa.

## Visão geral

O programa lê uma instância, constrói uma solução inicial viável e tenta minimizar a distância total percorrida pelos times, penalizando violações de restrições. A função objetivo considera:

- distância total viajada por todos os times;
- limite de jogos consecutivos em casa;
- limite de jogos consecutivos fora;
- diferença máxima entre mandos no turno.

Internamente, cada partida é armazenada em uma matriz `time_rodada[time][rodada]`:

- valor positivo: o time joga em casa contra o adversário indicado;
- valor negativo: o time joga fora contra o adversário indicado em módulo.

## Estrutura do repositório

```text
.
├── README.md
├── códigos/
│   ├── tcc2.cpp                 # implementação principal
│   ├── header_tcc2.h           # estruturas e assinaturas
│   ├── facilitador.py          # conversão de tabela oficial para ids numéricos
│   ├── instancias/
│   │   ├── feminina/
│   │   ├── masculina/
│   │   └── resultados/
│   └── output/
├── Modelo de Código e Leitura.zip
└── Modelo de Leitura de Instâncias.docx
```

## Arquivos principais

### `códigos/tcc2.cpp`

Arquivo principal do projeto. Implementa:

- leitura das instâncias;
- leitura de tabelas oficiais já prontas;
- geração da solução inicial pelo método do polígono;
- movimentos de vizinhança:
	- inversão de mando;
	- permuta de rodadas;
	- permuta de times;
- melhoria via Simulated Annealing;
- laço principal de ILS com perturbação e refinamento;
- cálculo da função objetivo e verificação da consistência da tabela.

Os parâmetros globais atualmente definidos no código foram calibrados previamente e incluem, entre outros:

- taxa de resfriamento;
- número de iterações do SA;
- temperatura inicial e final;
- intensidade das perturbações do ILS;
- tempo máximo de execução.

### `códigos/header_tcc2.h`

Declara variáveis globais, constantes, a estrutura `st_solucao` e os protótipos das funções.

### `códigos/facilitador.py`

Script auxiliar para transformar tabelas oficiais originalmente descritas com nomes de times em arquivos no formato numérico esperado pelo leitor de `tabela-oficial-*.txt`.

Observação: o caminho base desse script está fixo para um diretório local específico, então ele precisa ser ajustado antes de ser reutilizado em outro ambiente.

## Instâncias

As instâncias são o ponto central deste repositório.

### Organização

As instâncias ficam em `códigos/instancias/`, separadas por modalidade:

- `códigos/instancias/feminina/`
- `códigos/instancias/masculina/`

Em cada modalidade há dois tipos de arquivo:

- `dados-oficiais/dados-oficiais-XX-YY.txt`: instância completa usada pelo algoritmo;
- `tabela-oficial-XX-YY.txt`: tabela real da temporada, codificada numericamente.

### Disponibilidade por temporada

Situação atual dos arquivos versionados:

| Modalidade | `dados-oficiais` | `tabela-oficial` |
| --- | --- | --- |
| Feminina | 21-22, 22-23, 23-24 | 21-22, 22-23, 23-24, 24-25 |
| Masculina | 21-22, 22-23, 23-24 | 21-22, 22-23, 23-24, 24-25 |

Em outras palavras: as tabelas oficiais de 24-25 já estão no repositório, mas os arquivos `dados-oficiais-24-25.txt` ainda não estão presentes.

Isso importa porque o `main` atual está configurado para executar testes em lote para `22-23`, `23-24` e `24-25`. Sem adicionar os arquivos `dados-oficiais-24-25.txt`, essa execução em lote não fecha para a última temporada.

### Formato de `dados-oficiais-XX-YY.txt`

Esse é o formato realmente consumido por `ler_instancia()`.

Estrutura esperada:

1. número de times `n`;
2. `n` linhas com os nomes dos times;
3. matriz `n x n` de distâncias;
4. três inteiros finais com as restrições:
	 - `max_home`
	 - `max_away`
	 - `dif_home_away`

Exemplo esquemático:

```text
4
Time_A
Time_B
Time_C
Time_D
0 10 20 30
10 0 15 25
20 15 0 12
30 25 12 0
3
3
1
```

Interpretação dos três últimos valores:

- `max_home`: máximo de jogos consecutivos em casa;
- `max_away`: máximo de jogos consecutivos fora;
- `dif_home_away`: diferença máxima aceitável entre jogos em casa e fora ao final do turno.

Se `dif_home_away` for negativo, o código desconsidera essa restrição.

### Formato de `tabela-oficial-XX-YY.txt`

Esses arquivos guardam a tabela oficial de uma temporada já convertida para ids numéricos. Cada linha representa uma partida no formato:

```text
time_casa time_fora
```

As partidas aparecem em sequência de rodadas. Para uma instância com `n` times:

- cada rodada contém `n / 2` linhas;
- o campeonato completo contém `2 x (n - 1)` rodadas.

Exemplo:

```text
1 11
3 12
5 7
8 6
10 4
9 2
```

No exemplo acima, se `n = 12`, as 6 linhas correspondem a uma rodada completa.

Esses arquivos podem ser usados como referência para comparar a solução oficial com a solução produzida pelo algoritmo. A leitura é feita por `ler_tabela_resultado()`.

## Algoritmo

O fluxo principal implementado em `tcc2.cpp` é:

1. ler a instância;
2. gerar uma solução inicial pelo método do polígono;
3. aplicar Simulated Annealing para refinamento local;
4. aplicar ILS com perturbações e novas buscas locais até o limite de tempo;
5. validar a consistência da tabela final;
6. registrar métricas da melhor solução encontrada.

As perturbações e vizinhanças usadas são simples e diretas:

- troca de mando de uma partida, espelhando o returno;
- troca de duas rodadas do turno, junto com as rodadas correspondentes do returno;
- troca da identidade de dois times ao longo da tabela.

## Compilação

O projeto é essencialmente um único arquivo C++ com um header auxiliar. Em um ambiente com `g++`, uma compilação direta costuma ser suficiente:

```bash
cd códigos
g++ tcc2.cpp -O2 -std=c++17 -o tcc2.exe
```

Também é possível compilar com outro compilador C++ equivalente, desde que o padrão aceito seja compatível com o código atual.

## Execução

Importante: os caminhos do código são relativos à pasta `códigos`. Portanto, execute o binário com o diretório de trabalho posicionado nessa pasta.

### Modo atual do `main`

No estado atual do repositório, o `main` está configurado para execuções de teste em lote:

- modalidades: `masculina` e `feminina`;
- temporadas: `22-23`, `23-24` e `24-25`;
- 10 execuções por instância.

Durante esse processo, o programa escreve um CSV chamado `resultados-exec-teste.csv` com as colunas:

- `EDICAO`
- `TEMPORADA`
- `FO`
- `DIST TOTAL`
- `TEMPO MELHOR FO`

### Execução de uma instância específica

O próprio arquivo `tcc2.cpp` já traz blocos comentados para dois usos alternativos:

- execução normal de uma única instância, escolhendo `EDICAO` e `TEMPORADA` nas macros do código;
- execução voltada para calibração com irace, lendo parâmetros pela linha de comando.

Para rodar apenas uma instância, o caminho mais simples hoje é:

1. ajustar as macros `EDICAO` e `TEMPORADA`;
2. comentar o bloco de execuções de teste;
3. reativar o bloco de execução normal com `ler_instancia()`.

## Saídas geradas

As principais saídas do código atual são:

- impressão no terminal de informações da instância e da melhor solução;
- arquivo `resultados-exec-teste.csv` nas execuções em lote;
- funções auxiliares para gravar tabelas e soluções, algumas atualmente não utilizadas.

Há também referências no código a caminhos de calibração e resultados históricos, mas nem todas essas pastas estão presentes na versão pública do repositório.

## Limitações e observações práticas

- O `main` atual não está em modo "produção" para uma única instância; ele está preparado para um experimento em lote.
- As instâncias `dados-oficiais-24-25.txt` ainda não estão no repositório, embora as tabelas oficiais 24-25 já estejam.
- O script `facilitador.py` depende de caminho absoluto local e precisa de ajuste para reuso.
- Existe um executável versionado em `códigos/output/`, mas ele deve ser tratado como artefato auxiliar, não como substituto do código-fonte.

## Resumo

Este repositório reúne uma implementação heurística para otimização de calendários esportivos e um conjunto de instâncias reais derivadas da Superliga masculina e feminina. O ponto mais importante para quem quiser reutilizar o projeto é entender bem a pasta `códigos/instancias/`: os arquivos `dados-oficiais` são a entrada do algoritmo, enquanto os arquivos `tabela-oficial` servem como referência histórica para comparação e análise.