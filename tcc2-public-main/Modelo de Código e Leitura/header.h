#define MAX_TIMES 30

// Variáveis Principais

int num_times;
int nome_time;
char nome_cidade[50]; // talvez nao precise
int dist[MAX_TIMES][MAX_TIMES];

// Variáveis Auxiliares

int num_rodadas;

// Definições de Restrições

int max_home;
int max_away;
int dif_home_away;

// Solução

typedef struct d_sol
{
    int funObj;
    int time_rodada[MAX_TIMES][(MAX_TIMES - 1) * 2]; // guarda a tabela do campeonato
} st_solucao;

// Métodos Principais

void ler_instancia(char *arq);
void calc_FO(st_solucao &s);

// Métodos Auxiliares

void escreve_instancia();
