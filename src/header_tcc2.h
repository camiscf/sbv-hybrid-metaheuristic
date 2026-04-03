#define MAX_TIMES 31

// VARIÁVEIS DE LEITURA
int num_times;
char nome_time[MAX_TIMES][50];
int dist[MAX_TIMES][MAX_TIMES];
int max_iter;
int max_home;
int max_away;
int dif_home_away;

// VARIÁVEIS AUXILIARES
int num_rodadas;
int ultima_viagem[MAX_TIMES];
int dist_total_time[MAX_TIMES];
int times_ord_dist[MAX_TIMES];
// int qtd_times_a_jogar_turno[MAX_TIMES];
int time_jogou_turno_e_returno[MAX_TIMES][MAX_TIMES];
int time_jogou_rodada[MAX_TIMES][(MAX_TIMES - 1) * 2];
int qtd_times_a_jogar_rodada[MAX_TIMES];
int peso_qtd_times_a_jogar[MAX_TIMES];
int ord_base[MAX_TIMES];
int jogos_consec[MAX_TIMES][(MAX_TIMES - 1) * 2];

int poligono_times[MAX_TIMES];

typedef struct d_sol
{
    int funObj;
    int time_rodada[MAX_TIMES][(MAX_TIMES - 1) * 2]; // guarda o adversário do time x na rodada y -> num positivo joga em casa (rodada e times comecam em 1)
    int distancia_time[MAX_TIMES];
    int total_dist;
    int tempo_melhor_fo;
    int dif_mando_campo[MAX_TIMES];
} st_solucao;

void executar_otimizacao(st_solucao &s);
void executar_leitura_tabela_resultado();

// Métodos de Leitura
void ler_instancia(const char *arq);
void ler_tabela_resultado(const char *arq, st_solucao &s);

// Métodos de Escrita
void escreve_resultado_solucao(st_solucao &s);
void escreve_instancia();
void escrevePoligono();
void escreve_times_dist_ordenados();
void escreve_solucao_tabela_teste(st_solucao &s, const char *caminho, const char *edicao, const char *temporada, int cabecalho);
void escreve_sol_final_rodadas(st_solucao &s);
void escreve_solucao_detalhada_arquivo(st_solucao &s, const char *caminho, const char *edicao, const char *temporada, const char *metodo);

// Métodos do ILS
void iteratedLocalSearch(st_solucao &s);
void geraSolucaoInicial(st_solucao &s);
void geraPoligono(st_solucao &s);
void geraTabelaPoligono(st_solucao &s);
void rotacionaPoligono();
void perturbarSolucao(double omega, double teta, double beta, st_solucao &s);
void perturbaPoligono(st_solucao &s);
void inverte_mando(st_solucao &s);
void permuta_rodada(st_solucao &s);
void permuta_times(st_solucao &s);
void calcFO(st_solucao &s);

// Métodos de Busca Local
void simulateAnnealing(double tx_resf, int num_iter, int tp_inicial, double tp_final, st_solucao &s, clock_t h_inicial);

// Métodos de Ordenação
// void ordena_times_distancia(int esq, int dir);
// int particiona_distancia(int esq, int dir);
// int particiona_possibilidade_rodada(int esq, int dir);
// void troca_distancia(int i, int j);
// void troca_rodada(int i, int j);

// Métodos Auxiliares
void testa_tabela(st_solucao &s);
void copiaSolucao(st_solucao &s, st_solucao &s_aux);
void verifica_tabela_solucao(st_solucao &s);

// Métodos do GA + RVND
void geneticAlgorithm(st_solucao &s_best);
void crossover(st_solucao &p1, st_solucao &p2, st_solucao &child);
void repair(st_solucao &child);
void rvnd(st_solucao &s);
int torneio(st_solucao pop[], int n, int k);
void sort_populacao(st_solucao pop[], int n);
