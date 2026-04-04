#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "header_tcc2.h"

#define MAX(X, Y) ((X > Y) ? X : Y)
#define MIN(X, Y) ((X < Y) ? X : Y)
#define MOD(X) ((X > 0) ? X : -X)
#define MANDO(X) ((X > 0) ? 1 : -1)
#define ROUND(X) ((X < 1) ? 1 : X)
#define PESO 100000

// VARIAVEIS GLOBAIS PARA SA
// double TEMP_FIN = 0.01;
// int TEMPERATURA_INICIAL = 100;
// int NUMERO_ITERACOES = 10;
// double TAXA_RESFRIAMENTO = 0.9;

// VARIAVEIS GLOBAIS PARA ILS
// int MAX_EXEC = 50000;

// VARIAVEIS GLOBAIS PARA CALIBRACAO
// double PERTURBA_RODADA[] = {0.25, 0.5, 0.75};
// double PERTURBA_MANDO[] = {0.25, 0.5, 0.75};
// double PERTURBA_TIMES[] = {0.25, 0.5, 0.75};

// ######### VARIAVEIS GLOBAIS PARA CALIBRACAO IRACE #########

double TAXA_RESFRIAMENTO = 0.9362;
int NUMERO_ITERACOES = 500;
int TEMPERATURA_INICIAL = 100;
double PERTURBA_RODADA = 0.3802;
double PERTURBA_MANDO = 0.8537;
double PERTURBA_TIMES = 0.3939;
double TEMP_FIN = 0.01;

double tempo_execucao = 300; // segundos

// ######### PARÂMETROS DO GA #########

#define POP_SIZE 50
#define TORNEIO_K 3
#define PROB_MUTACAO 0.20
#define FRAC_ELITE 0.30

st_solucao populacao[POP_SIZE];
st_solucao nova_populacao[POP_SIZE];

// ######### PARÂMETROS DO Q-LEARNING (QVND) #########

#define Q_ESTADOS 81   // 3^4 features discretizadas
#define Q_ACOES 3      // 3 vizinhanças
#define Q_ALPHA 0.1    // taxa de aprendizado
#define Q_GAMMA_RL 0.9 // fator de desconto (nome diferente de gamma do SA)
#define Q_EPSILON_INIT 0.2
#define Q_EPSILON_MIN 0.05

double tabela_Q[Q_ESTADOS][Q_ACOES]; // persiste entre gerações
double q_epsilon = Q_EPSILON_INIT;   // decai ao longo das gerações

// ######### PARÂMETROS DA MULTIMINERAÇÃO #########

#define POOL_ELITE_SIZE 100

st_solucao pool_elite[POOL_ELITE_SIZE];
int pool_count = 0;
int mineracao_ativa = 0;              // 0 = Fase 1, 1 = Fase 2
int mando_freq[MAX_TIMES][MAX_TIMES]; // mando_freq[i][j] = vezes que i jogou em casa contra j nas elite

#define PASTA "../instancias"
#define CAMINHO_ARQUIVO_DADOS "/dados-oficiais-"
#define CAMINHO_ARQUIVO_RESULTADOS "/resultados-calib/calib-"
#define CAMINHO_ARQUIVO_TABELA_OFICIAL "/tabela-oficial-"
#define TXT ".txt"
#define SOL ".sol"

#define EDICAO "/feminina"
#define TEMPORADA "22-23"

int main(int argc, char *argv[])
{
    srand(time(0));

    // Uso: ./tcc2 <edicao> <temporada> [num_execucoes] [metodo]
    // Exemplo: ./tcc2 masculina 21-22 1 ga
    // metodo: "ils" (default), "ga", "qvnd", "gam" (GA+RVND+Min), "qvndm" (GA+QVND+Min)
    // Sem argumentos: roda todas as 8 instâncias × 10 execuções (ILS)

    if (argc >= 3)
    {
        const char *edicao = argv[1];
        const char *temporada = argv[2];
        int num_exec = (argc >= 4) ? atoi(argv[3]) : 1;
        const char *metodo = (argc >= 5) ? argv[4] : "ils";

        char caminho[200];
        snprintf(caminho, sizeof(caminho), "../instancias/%s%s%s.txt", edicao, CAMINHO_ARQUIVO_DADOS, temporada);
        printf("Caminho do arquivo: %s\n", caminho);
        ler_instancia(caminho);
        escreve_instancia();

        int cabecalho = 1;
        for (int k = 0; k < num_exec; k++)
        {
            st_solucao s;
            if (strcmp(metodo, "ga") == 0)
                geneticAlgorithm(s, 0, 0);
            else if (strcmp(metodo, "qvnd") == 0)
                geneticAlgorithm(s, 1, 0);
            else if (strcmp(metodo, "gam") == 0)
                geneticAlgorithm(s, 0, 1);
            else if (strcmp(metodo, "qvndm") == 0)
                geneticAlgorithm(s, 1, 1);
            else
                iteratedLocalSearch(s);

            // CSV por método
            char csv_name[100];
            snprintf(csv_name, sizeof(csv_name), "resultados-%s.csv", metodo);
            const char *csv = csv_name;
            escreve_solucao_tabela_teste(s, csv, edicao, temporada, cabecalho);
            escreve_solucao_detalhada_arquivo(s, csv, edicao, temporada, metodo);
            cabecalho = 0;
        }
    }
    else
    {
        // Modo batch: todas as instâncias × 10 execuções
        const char *ed[] = {"masculina", "feminina"};
        const char *temp[] = {"21-22", "22-23", "23-24", "24-25"};
        char caminho_teste[200];
        int cabecalho = 1;

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                snprintf(caminho_teste, sizeof(caminho_teste), "../instancias/%s%s%s.txt", ed[i], CAMINHO_ARQUIVO_DADOS, temp[j]);
                printf("Caminho do arquivo: %s\n", caminho_teste);
                ler_instancia(caminho_teste);
                escreve_instancia();

                for (int k = 0; k < 10; k++)
                {
                    st_solucao s;
                    iteratedLocalSearch(s);
                    escreve_solucao_tabela_teste(s, "resultados-exec-teste.csv", ed[i], temp[j], cabecalho);
                    cabecalho = 0;
                }
            }
        }
    }

    return 0;
}

void executar_otimizacao(st_solucao &s)
{

    st_solucao s_melhor, s_calib;
    int verif, cabecalho;
    int foBest = 1000000;

    iteratedLocalSearch(s);
}

void executar_leitura_tabela_resultado()
{
    escreve_instancia();

    const char *complemento_arq_res = "/tabela-oficial-";
    // const char *caminho = PASTA EDICAO CAMINHO_ARQUIVO_RESULTADOS TEMPORADA TXT;
    const char *caminho = PASTA EDICAO CAMINHO_ARQUIVO_TABELA_OFICIAL TEMPORADA TXT;
    printf("Caminho do arquivo: %s\n", caminho);

    st_solucao s_res;

    ler_tabela_resultado(caminho, s_res);
    calcFO(s_res);
    escreve_resultado_solucao(s_res);
}

// MÉTODOS DE LEITURA

void ler_instancia(const char *arq)
{
    FILE *f = fopen(arq, "r");

    fscanf(f, "%d", &num_times);

    num_rodadas = (num_times - 1) * 2;
    memset(&dist_total_time, 0, sizeof(dist_total_time));
    for (int i = 1; i <= num_times; i++)
    {
        fscanf(f, "%s", &nome_time[i]);
    }
    for (int i = 1; i <= num_times; i++)
    {
        for (int j = 1; j <= num_times; j++)
        {
            fscanf(f, "%d", &dist[i][j]);
            dist_total_time[i] += dist[i][j];
        }
        // times_ord_dist[i] = i;
        // qtd_times_a_jogar_turno[i] = num_times - 1;
        ord_base[i] = i;
    }
    fscanf(f, "%d %d %d", &max_home, &max_away, &dif_home_away);
}

void ler_tabela_resultado(const char *arq, st_solucao &s)
{
    FILE *f = fopen(arq, "r");
    int time1, time2;

    for (int i = 1; i <= num_rodadas; i++)
    {
        for (int j = 1; j <= num_times / 2; j++)
        {
            fscanf(f, "%d %d", &time1, &time2);
            s.time_rodada[time1][i] = time2;
            s.time_rodada[time2][i] = -time1;
        }
    }

    printf("Lendo arquivo %s\n", arq);
    fclose(f);
}

void iteratedLocalSearch(st_solucao &s)
{
    st_solucao s_melhor, s_aux;
    int melhor_fo;

    clock_t h, *h_aux;
    double tempo = 0;

    const clock_t CLOCK_INICIAL = clock();
    h = clock();

    geraSolucaoInicial(s);
    simulateAnnealing(TAXA_RESFRIAMENTO, NUMERO_ITERACOES, TEMPERATURA_INICIAL, TEMP_FIN, s, CLOCK_INICIAL);
    // calcFO(s);
    memcpy(&s_melhor, &s, sizeof(s));
    melhor_fo = s.funObj;
    printf("Melhor FO = %d\n", melhor_fo);
    while (tempo <= tempo_execucao)
    {
        perturbarSolucao(PERTURBA_RODADA, PERTURBA_MANDO, PERTURBA_TIMES, s);
        simulateAnnealing(TAXA_RESFRIAMENTO, NUMERO_ITERACOES, TEMPERATURA_INICIAL, TEMP_FIN, s, CLOCK_INICIAL);
        // calcFO(s);
        // printf("FO = %d\n", s.funObj);
        if (s.funObj < melhor_fo)
        {
            s.tempo_melhor_fo = (double)(clock() - h) / CLOCKS_PER_SEC;
            memcpy(&s_melhor, &s, sizeof(s));
            melhor_fo = s.funObj;
            printf("Melhor FO = %d TEMPO = %d\n", melhor_fo, s.tempo_melhor_fo);
            verifica_tabela_solucao(s);
        }
        tempo = (double)(clock() - h) / CLOCKS_PER_SEC;
    }
    memcpy(&s, &s_melhor, sizeof(s));
    verifica_tabela_solucao(s);
    printf("%d\n", s.funObj);
}

void geraSolucaoInicial(st_solucao &s)
{
    geraPoligono(s);
    // escrevePoligono();
    geraTabelaPoligono(s);
}

void geraPoligono(st_solucao &s)
{
    int times_para_sortear = num_times;
    int posicao_poligono = 0;

    int time_sorteado[num_times]; // era num_times-1 (bug OOB no original)
    memset(&time_sorteado, 0, sizeof(time_sorteado)); // define que nenhum time foi sorteado

    while (times_para_sortear > 0)
    {
        int time = 1 + (rand() % num_times);
        while (time_sorteado[time - 1] == 1)
            time = 1 + (rand() % num_times);

        poligono_times[posicao_poligono] = time;
        posicao_poligono++;
        times_para_sortear--;
        time_sorteado[time - 1] = 1;
    }
}

void geraTabelaPoligono(st_solucao &s)
{
    int mando_campo_time_base = 1; // inicia jogando em casa

    for (int rodada = 1; rodada <= num_rodadas / 2; rodada++)
    {
        // TURNO
        s.time_rodada[poligono_times[0]][rodada] = poligono_times[1] * mando_campo_time_base;
        s.time_rodada[poligono_times[1]][rodada] = poligono_times[0] * mando_campo_time_base * -1;

        // RETURNO
        s.time_rodada[poligono_times[0]][rodada + (num_rodadas / 2)] = poligono_times[1] * mando_campo_time_base * -1;
        s.time_rodada[poligono_times[1]][rodada + (num_rodadas / 2)] = poligono_times[0] * mando_campo_time_base;

        mando_campo_time_base *= -1; // muda o mando para a prox rodada

        int mando_campo_times_poligono = 1;      // times nas posicoes pares comecam em casa
        for (int i = 2; i <= num_times / 2; i++) // comeca no terceiro time -> diminuir 1 das equações do tcc (vetor do poligono comeca em 0)
        {
            int adv = num_times - i + 1;

            // TURNO
            s.time_rodada[poligono_times[i]][rodada] = poligono_times[adv] * mando_campo_times_poligono;
            s.time_rodada[poligono_times[adv]][rodada] = poligono_times[i] * mando_campo_times_poligono * -1;

            // RETURNO
            s.time_rodada[poligono_times[i]][rodada + (num_rodadas / 2)] = poligono_times[adv] * mando_campo_times_poligono * -1;
            s.time_rodada[poligono_times[adv]][rodada + (num_rodadas / 2)] = poligono_times[i] * mando_campo_times_poligono;

            mando_campo_times_poligono *= -1;
        }
        // printf("Rodada = %d\n", rodada);
        // printf("Rotacionando\n");
        rotacionaPoligono();
        // escrevePoligono();
    }
}

void rotacionaPoligono()
{
    const int time_pos2 = poligono_times[1];
    int aux;

    for (int i = 1; i < num_times - 1; i++) // time base fixo
        poligono_times[i] = poligono_times[i + 1];

    poligono_times[num_times - 1] = time_pos2;
}

void perturbarSolucao(double omega, double teta, double beta, st_solucao &s)
{
    int num_perturbacoes_rodada = (int)(omega * num_rodadas / 2);
    int num_perturbacoes_mando = (int)(teta * num_rodadas * num_times / 2);
    int num_perturbacoes_times = (int)(beta * num_times / 2);

    for (int i = 0; i < num_perturbacoes_rodada; i++)
    {
        permuta_rodada(s);
    }

    for (int i = 0; i < num_perturbacoes_mando; i++)
    {
        inverte_mando(s);
    }

    for (int i = 0; i < num_perturbacoes_times; i++)
    {
        permuta_times(s);
    }
}

void inverte_mando(st_solucao &s)
{
    // sortear uma partida de uma rodada aleatoria e trocar o seu mando (fazer tambem para o returno)

    int rodada = 1 + (rand() % (num_rodadas / 2));
    int time = 1 + (rand() % num_times);
    int adv = MOD(s.time_rodada[time][rodada]);

    s.time_rodada[time][rodada] *= -1;
    s.time_rodada[adv][rodada] *= -1;
    s.time_rodada[time][rodada + (num_rodadas / 2)] *= -1;
    s.time_rodada[adv][rodada + (num_rodadas / 2)] *= -1;
}

void permuta_rodada(st_solucao &s)
{
    int rodada1, rodada2, aux;

    rodada1 = 1 + (rand() % (num_rodadas / 2));
    do
    {
        rodada2 = 1 + (rand() % (num_rodadas / 2));
    } while (rodada1 == rodada2);

    for (int i = 1; i <= num_times; i++)
    {
        // turno
        aux = s.time_rodada[i][rodada2];
        s.time_rodada[i][rodada2] = s.time_rodada[i][rodada1];
        s.time_rodada[i][rodada1] = aux;
        // returno
        aux = s.time_rodada[i][rodada2 + (num_rodadas / 2)];
        s.time_rodada[i][rodada2 + (num_rodadas / 2)] = s.time_rodada[i][rodada1 + (num_rodadas / 2)];
        s.time_rodada[i][rodada1 + (num_rodadas / 2)] = aux;
    }
}

void permuta_times(st_solucao &s)
{
    int time1 = 1 + (rand() % num_times);
    int time2 = 1 + (rand() % num_times);
    while (time1 == time2)
    {
        time2 = 1 + (rand() % num_times);
    }

    for (int i = 1; i <= num_rodadas / 2; i++)
    {
        // turno
        int adv_time1 = s.time_rodada[time1][i];
        int adv_time2 = s.time_rodada[time2][i];
        if (adv_time1 != time2 && adv_time2 != time1)
        {
            // printf("\nAntes das trocas:\ntime1 = %d, time2 = %d, time1_rodada = %d, time2_rodada = %d\n", time1, time2, adv_time1, adv_time2);
            // printf("Adv_time1 = %d, Adv_time2 = %d\n", s.time_rodada[MOD(adv_time1)][i], s.time_rodada[MOD(adv_time2)][i]);
            s.time_rodada[time1][i] = adv_time2;
            s.time_rodada[time2][i] = adv_time1;
            s.time_rodada[MOD(adv_time1)][i] = MANDO(s.time_rodada[MOD(adv_time1)][i]) * time2;
            s.time_rodada[MOD(adv_time2)][i] = MANDO(s.time_rodada[MOD(adv_time2)][i]) * time1;
            // printf("Depois das trocas:\ntime1 = %d, time2 = %d, time1_rodada = %d, time2_rodada = %d\n", time1, time2, s.time_rodada[time1][i], s.time_rodada[time2][i]);
            // printf("Adv_time1 = %d, Adv_time2 = %d\n", s.time_rodada[MOD(adv_time1)][i], s.time_rodada[MOD(adv_time2)][i]);
            // returno
            s.time_rodada[time1][i + (num_rodadas / 2)] = s.time_rodada[time1][i] * -1;
            s.time_rodada[time2][i + (num_rodadas / 2)] = s.time_rodada[time2][i] * -1;
            s.time_rodada[MOD(adv_time1)][i + (num_rodadas / 2)] = s.time_rodada[MOD(adv_time1)][i] * -1;
            s.time_rodada[MOD(adv_time2)][i + (num_rodadas / 2)] = s.time_rodada[MOD(adv_time2)][i] * -1;
        }
        else
        {
            s.time_rodada[time1][i] *= -1;
            s.time_rodada[time2][i] *= -1;
            s.time_rodada[time1][i + (num_rodadas / 2)] *= -1;
            s.time_rodada[time2][i + (num_rodadas / 2)] *= -1;
        }
    }
    verifica_tabela_solucao(s);
    // escreve_solucao(s, 1);
}

void escrevePoligono()
{
    printf("Poligono\n");
    for (int i = 0; i < num_times; i++)
    {
        printf("poligono_times[%d] = %s\n", i, nome_time[poligono_times[i]]);
    }
}

void simulateAnnealing(double tx_resf, int num_iter, int tp_inicial, double tp_final, st_solucao &s, clock_t h_inicial)
{
    st_solucao s_aux, s_viz;
    float t = tp_inicial, x;
    int delta, metodo, aux1, aux2;
    clock_t h_aux;
    double tempo_aux;
    memcpy(&s_aux, &s, sizeof(s));

    while (t > tp_final)
    {
        for (int iter = 0; iter < num_iter; iter++)
        {
            memcpy(&s_viz, &s_aux, sizeof(s_aux));

            metodo = rand() % 3;
            switch (metodo)
            {
            case 0:
                inverte_mando(s_viz);
                break;
            case 1:
                permuta_rodada(s_viz);
                break;
            case 2:
                permuta_times(s_viz);
            default:
                break;
            }

            calcFO(s_viz);
            delta = s_viz.funObj - s_aux.funObj;
            if (delta < 0)
            {
                memcpy(&s_aux, &s_viz, sizeof(s_viz));
                if (s_viz.funObj < s.funObj)
                {
                    memcpy(&s, &s_viz, sizeof(s_viz));
                    h_aux = clock() - h_inicial; // era h_mel
                    s.tempo_melhor_fo = (double)h_aux / CLOCKS_PER_SEC;
                }
            }
            else
            {
                x = rand() % 1001;
                x = x / 1000.0;
                if (x < exp(-delta / t))
                {
                    memcpy(&s_aux, &s_viz, sizeof(s_viz));
                }
            }
            h_aux = clock() - h_inicial;
            tempo_aux = (double)h_aux / CLOCKS_PER_SEC;
            if ((tempo_aux >= tempo_execucao))
            {
                goto END;
            }
        }
        t = tx_resf * t;
    }
END:;
    calcFO(s);
}

void calcFO(st_solucao &s)
{
    s.funObj = 0;
    s.total_dist = 0;
    memset(&s.distancia_time, 0, sizeof(s.distancia_time));
    memset(&jogos_consec, 0, sizeof(jogos_consec));
    memset(&s.dif_mando_campo, 0, sizeof(s.dif_mando_campo));
    for (int i = 1; i <= num_times; i++)
    {
        ultima_viagem[i] = i;
        for (int j = 1; j <= num_rodadas; j++)
        {
            if (s.time_rodada[i][j] > 0) // joga em casa
            {
                s.distancia_time[i] += dist[ultima_viagem[i]][i];
                ultima_viagem[i] = i;

                if (s.time_rodada[i][j - 1] < 0)
                {
                    jogos_consec[i][j] = 1;
                }
                else
                {
                    jogos_consec[i][j] = jogos_consec[i][j - 1] + 1;

                    if (jogos_consec[i][j] > max_home)
                    {
                        s.funObj += PESO;
                    }
                }
                s.dif_mando_campo[i]++;
            }
            else
            {
                s.distancia_time[i] += dist[ultima_viagem[i]][-s.time_rodada[i][j]];
                ultima_viagem[i] = s.time_rodada[i][j] * (-1);

                if (s.time_rodada[i][j - 1] > 0)
                {
                    jogos_consec[i][j] = -1;
                }
                else
                {
                    jogos_consec[i][j] = jogos_consec[i][j - 1] - 1;

                    if (MOD(jogos_consec[i][j]) > max_away)
                    {
                        s.funObj += PESO;
                    }
                }
                s.dif_mando_campo[i]--;
            }

            if (j == num_rodadas / 2)
            {
                if ((MOD(s.dif_mando_campo[i]) > dif_home_away) && (dif_home_away >= 0)) // se for negativo nao há essa restrição
                {
                    s.funObj += PESO * 2; // turno e returno
                }
            }
        }
        s.funObj += s.distancia_time[i];
        s.total_dist += s.distancia_time[i];
    }
}

void copiaSolucao(st_solucao &s, st_solucao &s_aux)
{
    memcpy(&s_aux, &s, sizeof(s));
}

void testa_tabela(st_solucao &s)
{
    for (int i = 1; i <= num_rodadas; i++)
    {
        for (int j = 1; j <= num_times; j++)
        {
            printf("%d -> %d ", j, s.time_rodada[j][i]);
        }
        printf("\n");
    }
}

void escreve_resultado_solucao(st_solucao &s)
{
    // calcFO(s);
    printf("\nFO = %d\nDist Total = %d\nTempo Melhor FO = %d\n", s.funObj, s.total_dist, s.tempo_melhor_fo);
}

void escreve_solucao_detalhada(st_solucao &s)
{
    printf("Numero de rodadas : %d\n", num_rodadas);
    printf("\n\n-------- Tabela ---------\n");
    for (int j = 1; j <= num_rodadas; j++)
    {
        printf("\nRodada %d:\n", j);
        for (int i = 1; i <= num_times; i++)
        {
            if (s.time_rodada[i][j] > 0)
            {
                printf("%s x %s\n", nome_time[i], nome_time[s.time_rodada[i][j]]);
            }
        }
    }
    printf("\n----- Distancia Percorrida por cada time -----\n\n");
    for (int i = 1; i <= num_times; i++)
    {
        printf("%s = %d\n", nome_time[i], s.distancia_time[i]);
    }
    printf("\nFO = %d\nDist Total = %d\nTempo Melhor FO = %d\n", s.funObj, s.total_dist, s.tempo_melhor_fo);
}

// Salva tabela detalhada em arquivo TXT
void escreve_solucao_detalhada_arquivo(st_solucao &s, const char *caminho, const char *edicao, const char *temporada, const char *metodo)
{
    char nome_arq[300];
    snprintf(nome_arq, sizeof(nome_arq), "solucao-%s-%s-%s.txt", metodo, edicao, temporada);

    FILE *f = fopen(nome_arq, "w");
    if (f == NULL)
    {
        printf("Erro ao abrir %s\n", nome_arq);
        return;
    }

    fprintf(f, "Metodo: %s\n", metodo);
    fprintf(f, "Instancia: %s %s\n", edicao, temporada);
    fprintf(f, "FO = %d\n", s.funObj);
    fprintf(f, "Dist Total = %d\n", s.total_dist);
    fprintf(f, "Tempo Melhor FO = %d\n\n", s.tempo_melhor_fo);

    fprintf(f, "-------- Tabela ---------\n");
    for (int j = 1; j <= num_rodadas; j++)
    {
        fprintf(f, "\nRodada %d:\n", j);
        for (int i = 1; i <= num_times; i++)
        {
            if (s.time_rodada[i][j] > 0)
            {
                fprintf(f, "  %s x %s\n", nome_time[i], nome_time[s.time_rodada[i][j]]);
            }
        }
    }

    fprintf(f, "\n----- Distancia por time -----\n\n");
    for (int i = 1; i <= num_times; i++)
    {
        fprintf(f, "%s = %d\n", nome_time[i], s.distancia_time[i]);
    }

    fclose(f);
    printf("Tabela salva em %s\n", nome_arq);
}

void escreve_solucao_tabela_teste(st_solucao &s, const char *caminho, const char *edicao, const char *temporada, int cabecalho)
{
    // const char *nome_arquivo = "./instancias/feminina/resultados-calib/calib-22-23.txt";

    FILE *arq = fopen(caminho, "a");
    if (arq == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    if (cabecalho == 1)
    {
        fprintf(arq, "EDICAO;");
        fprintf(arq, "TEMPORADA;");
        fprintf(arq, "FO;");
        fprintf(arq, "DIST TOTAL;");
        fprintf(arq, "TEMPO MELHOR FO\n");
    }

    fprintf(arq, "%s;", edicao);
    fprintf(arq, "%s;", temporada);
    fprintf(arq, "%d;", s.funObj);
    fprintf(arq, "%d;", s.total_dist);
    fprintf(arq, "%d\n", s.tempo_melhor_fo);

    fclose(arq);
}

// NÃO ESTÁ SENDO USADO
void escreve_sol_final_rodadas(st_solucao &s)
{
    FILE *sol = fopen("../instancias/res-melhor-feminino.txt", "w");
    if (sol == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    for (int j = 1; j <= num_rodadas; j++)
    {
        for (int i = 1; i <= num_times; i++)
        {
            if (s.time_rodada[i][j] > 0)
            {
                fprintf(sol, "%d %d\n", i, s.time_rodada[i][j]);
            }
        }
    }
    fclose(sol);
}

void escreve_instancia()
{
    printf("Numero de Times: %d\n", num_times);
    printf("Nome dos Times\n");
    for (int i = 1; i <= num_times; i++)
    {
        printf("%s\n", nome_time[i]);
    }
    printf("Distancia Cidades\n");
    for (int i = 1; i <= num_times; i++)
    {
        for (int j = 1; j <= num_times; j++)
        {
            printf("%d ", dist[i][j]);
        }
        printf("\n");
    }
    printf("\n%d %d %d\n", max_home, max_away, dif_home_away);
}

void verifica_tabela_solucao(st_solucao &s)
{
    // fazer para todas as possibilidades (usar matriz de 'jogou_turno' e 'jogou_returno'), etc
    // verificar se todos os times jogaram em todas as rodadas
    for (int i = 1; i <= num_times; i++)
    {
        int cont = 0;
        for (int j = 1; j <= num_rodadas; j++)
        {
            if (s.time_rodada[i][j] != 0)
            {
                cont++;
            }
        }
        if (cont != num_rodadas)
        {
            printf("**********\n**********\n**********\n**********\n**********\nTime %s nao jogou em todas as rodadas\n", nome_time[i]);
            break;
        }
    }

    // verificar se tem todos os jogos corretos
    for (int j = 1; j <= num_rodadas; j++)
    {
        int cont = 0;
        for (int i = 1; i <= num_times; i++)
        {
            if (s.time_rodada[i][j] > 0)
            {
                cont++;
            }
        }
        if (cont != num_times / 2)
        {
            printf("**********\n**********\n**********\n**********\n**********\nRodada %d nao tem todos os jogos corretos\n", j);
            break;
        }
    }

    // verificar se jogou contra todos os times no turno e returno de maneira espelhada
    memset(time_jogou_turno_e_returno, 0, sizeof(time_jogou_turno_e_returno));
    for (int j = 1; j <= num_rodadas / 2; j++)
    {
        for (int i = 1; i <= num_times; i++)
        {
            if ((MOD(s.time_rodada[i][j]) == MOD(s.time_rodada[i][j + (num_rodadas / 2)])))
            {
                time_jogou_turno_e_returno[i][MOD(s.time_rodada[i][j])] = 1;
            }
        }
    }

    for (int i = 1; i <= num_times; i++)
    {
        for (int j = 1; j <= num_times; j++)
        {
            if ((time_jogou_turno_e_returno[i][j] == 0) && (i != j))
            {
                printf("**********\n**********\n**********\n**********\n**********\nTime %s nao jogou em todas as rodadas\n", nome_time[i]);
                break;
            }
        }
    }

    // verificar se tem todos os mandos de campo corretos
    for (int j = 1; j <= num_rodadas; j++)
    {
        int cont = 0;
        for (int i = 1; i <= num_times; i++)
        {
            if (s.time_rodada[i][j] > 0)
            {
                cont++;
            }
            else
            {
                cont--;
            }
        }
        if (cont != 0)
        {
            printf("**********\n**********\n**********\n**********\n**********\nRodada %d nao tem todos os mandos de campo corretos\n", j);
            break;
        }
    }
}

// ######### GA + RVND #########

// Seleção por torneio: sorteia k indivíduos, retorna índice do melhor
int torneio(st_solucao pop[], int n, int k)
{
    int best = rand() % n;
    for (int i = 1; i < k; i++)
    {
        int cand = rand() % n;
        if (pop[cand].funObj < pop[best].funObj)
            best = cand;
    }
    return best;
}

// Ordena população por funObj (insertion sort — N=50, ok)
void sort_populacao(st_solucao pop[], int n)
{
    for (int i = 1; i < n; i++)
    {
        st_solucao key;
        memcpy(&key, &pop[i], sizeof(st_solucao));
        int j = i - 1;
        while (j >= 0 && pop[j].funObj > key.funObj)
        {
            memcpy(&pop[j + 1], &pop[j], sizeof(st_solucao));
            j--;
        }
        memcpy(&pop[j + 1], &key, sizeof(st_solucao));
    }
}

// RVND: busca local com vizinhanças em ordem aleatória, só aceita melhoras
// Tenta RVND_MAX_TENT movimentos por vizinhança antes de desistir
#define RVND_MAX_TENT 50

void rvnd(st_solucao &s)
{
    int ordem[3] = {0, 1, 2};

    // Fisher-Yates shuffle
    for (int i = 2; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = ordem[i];
        ordem[i] = ordem[j];
        ordem[j] = tmp;
    }

    int k = 0;
    while (k < 3)
    {
        int melhorou = 0;
        for (int tent = 0; tent < RVND_MAX_TENT; tent++)
        {
            st_solucao s_viz;
            memcpy(&s_viz, &s, sizeof(st_solucao));

            switch (ordem[k])
            {
            case 0:
                permuta_rodada(s_viz);
                break;
            case 1:
                inverte_mando(s_viz);
                break;
            case 2:
                permuta_times(s_viz);
                break;
            }

            calcFO(s_viz);

            if (s_viz.funObj < s.funObj)
            {
                memcpy(&s, &s_viz, sizeof(st_solucao));
                melhorou = 1;
                break;
            }
        }

        if (melhorou)
        {
            k = 0;
            // Re-shuffle
            for (int i = 2; i > 0; i--)
            {
                int j = rand() % (i + 1);
                int tmp = ordem[i];
                ordem[i] = ordem[j];
                ordem[j] = tmp;
            }
        }
        else
        {
            k++;
        }
    }
}

// ######### QVND — Q-Learning VND #########

// Inicializa tabela Q com zeros
void inicializa_tabela_Q()
{
    memset(tabela_Q, 0, sizeof(tabela_Q));
    q_epsilon = Q_EPSILON_INIT;
}

// Extrai estado discretizado da solução (4 features → 81 estados)
int extrair_estado(st_solucao &s, int ultima_viz)
{
    // Feature 1: distância normalizada (0, 1, 2)
    // Normalizar pelo máximo teórico: soma de todas as distâncias
    int dist_max = 0;
    for (int i = 1; i <= num_times; i++)
        dist_max += dist_total_time[i];
    if (dist_max == 0) dist_max = 1;
    double dist_norm = (double)s.total_dist / dist_max;
    int f1;
    if (dist_norm < 0.3) f1 = 0;
    else if (dist_norm < 0.6) f1 = 1;
    else f1 = 2;

    // Feature 2: viagens longas >1500km (0, 1, 2)
    int viagens_longas = 0;
    for (int i = 1; i <= num_times; i++)
    {
        int ultima = i;
        for (int j = 1; j <= num_rodadas; j++)
        {
            int destino;
            if (s.time_rodada[i][j] > 0)
                destino = i; // joga em casa
            else
                destino = MOD(s.time_rodada[i][j]); // joga fora
            if (dist[ultima][destino] > 1500)
                viagens_longas++;
            ultima = destino;
        }
    }
    int f2;
    if (viagens_longas <= 2) f2 = 0;
    else if (viagens_longas <= 5) f2 = 1;
    else f2 = 2;

    // Feature 3: máximo consecutivos fora (0, 1, 2)
    int max_consec_fora = 0;
    for (int i = 1; i <= num_times; i++)
    {
        int consec = 0;
        for (int j = 1; j <= num_rodadas; j++)
        {
            if (s.time_rodada[i][j] < 0)
            {
                consec++;
                if (consec > max_consec_fora)
                    max_consec_fora = consec;
            }
            else
            {
                consec = 0;
            }
        }
    }
    int f3;
    if (max_consec_fora <= 2) f3 = 0;
    else if (max_consec_fora <= 4) f3 = 1;
    else f3 = 2;

    // Feature 4: última vizinhança usada (0, 1, 2)
    int f4 = ultima_viz; // 0=permuta_rodada, 1=inverte_mando, 2=permuta_times

    return f1 * 27 + f2 * 9 + f3 * 3 + f4;
}

// Política ε-greedy: escolhe ação com maior Q ou aleatória com prob ε
int epsilon_greedy(int estado)
{
    double r = (double)(rand() % 1001) / 1000.0;
    if (r < q_epsilon)
    {
        return rand() % Q_ACOES; // exploração
    }
    else
    {
        // exploitation: ação com maior Q
        int melhor = 0;
        for (int a = 1; a < Q_ACOES; a++)
        {
            if (tabela_Q[estado][a] > tabela_Q[estado][melhor])
                melhor = a;
        }
        return melhor;
    }
}

// QVND: busca local guiada por Q-Learning
void qvnd(st_solucao &s)
{
    int ultima_viz = rand() % 3; // inicializa aleatoriamente
    int estado = extrair_estado(s, ultima_viz);

    int sem_melhora = 0;
    // Para quando 3 ações consecutivas não melhoram (análogo ao RVND)
    while (sem_melhora < Q_ACOES)
    {
        int acao = epsilon_greedy(estado);

        // Aplicar vizinhança com múltiplas tentativas (como RVND)
        int melhorou = 0;
        for (int tent = 0; tent < RVND_MAX_TENT; tent++)
        {
            st_solucao s_viz;
            memcpy(&s_viz, &s, sizeof(st_solucao));

            switch (acao)
            {
            case 0:
                permuta_rodada(s_viz);
                break;
            case 1:
                inverte_mando(s_viz);
                break;
            case 2:
                permuta_times(s_viz);
                break;
            }

            calcFO(s_viz);

            double recompensa = (s.funObj > s_viz.funObj) ? (double)(s.funObj - s_viz.funObj) : 0.0;
            int novo_estado = extrair_estado(s_viz, acao);

            // Atualização Q-Learning
            double max_q_next = tabela_Q[novo_estado][0];
            for (int a = 1; a < Q_ACOES; a++)
            {
                if (tabela_Q[novo_estado][a] > max_q_next)
                    max_q_next = tabela_Q[novo_estado][a];
            }
            tabela_Q[estado][acao] += Q_ALPHA * (recompensa + Q_GAMMA_RL * max_q_next - tabela_Q[estado][acao]);

            if (s_viz.funObj < s.funObj)
            {
                memcpy(&s, &s_viz, sizeof(st_solucao));
                estado = novo_estado;
                melhorou = 1;
                break;
            }
        }

        if (melhorou)
        {
            sem_melhora = 0;
        }
        else
        {
            sem_melhora++;
        }
    }
}

// ######### MULTIMINERAÇÃO #########

// Reset para nova execução
void reset_mineracao()
{
    pool_count = 0;
    mineracao_ativa = 0;
    memset(mando_freq, 0, sizeof(mando_freq));
}

// Atualiza pool com as melhores soluções vistas
void atualiza_pool_elite(st_solucao pop[], int num_elite)
{
    for (int i = 0; i < num_elite; i++)
    {
        if (pool_count < POOL_ELITE_SIZE)
        {
            memcpy(&pool_elite[pool_count], &pop[i], sizeof(st_solucao));
            pool_count++;
        }
        else
        {
            // Pool cheio: substituir a pior se a nova for melhor
            int pior = 0;
            for (int k = 1; k < POOL_ELITE_SIZE; k++)
                if (pool_elite[k].funObj > pool_elite[pior].funObj)
                    pior = k;
            if (pop[i].funObj < pool_elite[pior].funObj)
                memcpy(&pool_elite[pior], &pop[i], sizeof(st_solucao));
        }
    }
}

// Extrai frequência de mando por par de times no turno das elite
void minerar_padroes()
{
    memset(mando_freq, 0, sizeof(mando_freq));

    for (int s = 0; s < pool_count; s++)
    {
        for (int r = 1; r <= num_rodadas / 2; r++)
        {
            for (int t = 1; t <= num_times; t++)
            {
                if (pool_elite[s].time_rodada[t][r] > 0)
                {
                    int adv = pool_elite[s].time_rodada[t][r];
                    mando_freq[t][adv]++;
                }
            }
        }
    }

    mineracao_ativa = 1;
    printf("Mineracao: %d solucoes mineradas\n", pool_count);
}

// Crossover greedy por rodada: escolhe pai que minimiza conflitos de matchup
void crossover(st_solucao &p1, st_solucao &p2, st_solucao &child)
{
    int played[MAX_TIMES][MAX_TIMES]; // played[t][u] = 1 se t já enfrenta u no turno do filho
    memset(played, 0, sizeof(played));

    // Embaralhar ordem das rodadas para evitar viés
    int round_order[MAX_TIMES];
    for (int i = 0; i < num_rodadas / 2; i++)
        round_order[i] = i + 1;
    for (int i = num_rodadas / 2 - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = round_order[i];
        round_order[i] = round_order[j];
        round_order[j] = tmp;
    }

    for (int idx = 0; idx < num_rodadas / 2; idx++)
    {
        int r = round_order[idx];

        // Contar conflitos de cada pai para esta rodada
        int conf1 = 0, conf2 = 0;
        for (int t = 1; t <= num_times; t++)
        {
            int u1 = MOD(p1.time_rodada[t][r]);
            if (t < u1 && played[t][u1])
                conf1++;
            int u2 = MOD(p2.time_rodada[t][r]);
            if (t < u2 && played[t][u2])
                conf2++;
        }

        // Escolher pai com menos conflitos
        st_solucao *pai;
        if (conf1 < conf2)
            pai = &p1;
        else if (conf2 < conf1)
            pai = &p2;
        else if (mineracao_ativa)
        {
            // Empate: desempatar por padrões minerados (Fase 2)
            int score1 = 0, score2 = 0;
            for (int t = 1; t <= num_times; t++)
            {
                if (p1.time_rodada[t][r] > 0)
                    score1 += mando_freq[t][p1.time_rodada[t][r]];
                if (p2.time_rodada[t][r] > 0)
                    score2 += mando_freq[t][p2.time_rodada[t][r]];
            }
            pai = (score1 >= score2) ? &p1 : &p2;
        }
        else
            pai = (rand() % 2 == 0) ? &p1 : &p2;

        for (int t = 1; t <= num_times; t++)
        {
            child.time_rodada[t][r] = pai->time_rodada[t][r];
            child.time_rodada[t][r + num_rodadas / 2] = pai->time_rodada[t][r] * -1;
            int u = MOD(pai->time_rodada[t][r]);
            played[t][u] = 1;
            played[u][t] = 1;
        }
    }
}

// Contadores de crossover (diagnóstico)
int crossover_ok = 0, crossover_fail = 0;

// Reparo pós-crossover: se há conflitos, gera solução nova
void repair(st_solucao &child)
{
    // Verificar se há duplicatas no turno
    int played[MAX_TIMES][MAX_TIMES];
    memset(played, 0, sizeof(played));

    for (int r = 1; r <= num_rodadas / 2; r++)
    {
        for (int t = 1; t <= num_times; t++)
        {
            int u = MOD(child.time_rodada[t][r]);
            if (t < u)
                played[t][u]++;
        }
    }

    for (int t = 1; t <= num_times; t++)
    {
        for (int u = t + 1; u <= num_times; u++)
        {
            if (played[t][u] != 1)
            {
                crossover_fail++;
                geraSolucaoInicial(child);
                return;
            }
        }
    }
    crossover_ok++;
    // Tabela válida — nada a fazer
}

// Loop principal do GA
void geneticAlgorithm(st_solucao &s_best, int usar_qvnd, int usar_mineracao)
{
    clock_t h_inicio = clock();
    double tempo = 0;

    // Inicializar tabela Q se usando QVND
    if (usar_qvnd)
        inicializa_tabela_Q();

    // Inicializar mineração se ativa
    if (usar_mineracao)
        reset_mineracao();

    // Inicializar população
    for (int i = 0; i < POP_SIZE; i++)
    {
        geraSolucaoInicial(populacao[i]);
        calcFO(populacao[i]);
    }

    sort_populacao(populacao, POP_SIZE);
    memcpy(&s_best, &populacao[0], sizeof(st_solucao));
    s_best.tempo_melhor_fo = 0;
    printf("GA Init: melhor FO = %d\n", s_best.funObj);

    int geracao = 0;

    while (tempo < tempo_execucao)
    {
        // Elitismo: manter elite da geração anterior, gerar filhos para o resto
        int num_elite = (int)(FRAC_ELITE * POP_SIZE);
        int num_filhos = POP_SIZE - num_elite;

        for (int i = 0; i < num_filhos; i++)
        {
            int p1 = torneio(populacao, POP_SIZE, TORNEIO_K);
            int p2 = torneio(populacao, POP_SIZE, TORNEIO_K);
            while (p2 == p1)
                p2 = torneio(populacao, POP_SIZE, TORNEIO_K);

            crossover(populacao[p1], populacao[p2], nova_populacao[i]);
            repair(nova_populacao[i]);

            // Mutação (protegida na Fase 2 da mineração)
            double r = (double)(rand() % 1001) / 1000.0;
            if (r < PROB_MUTACAO)
            {
                int mov = rand() % 3;
                if (mineracao_ativa && usar_mineracao && mov == 0)
                {
                    // inverte_mando protegida: só inverte se mando atual NÃO é o frequente
                    int rod = 1 + (rand() % (num_rodadas / 2));
                    int t = 1 + (rand() % num_times);
                    int adv = MOD(nova_populacao[i].time_rodada[t][rod]);
                    int freq_atual = (nova_populacao[i].time_rodada[t][rod] > 0)
                                         ? mando_freq[t][adv]
                                         : mando_freq[adv][t];
                    int freq_inverso = (nova_populacao[i].time_rodada[t][rod] > 0)
                                           ? mando_freq[adv][t]
                                           : mando_freq[t][adv];
                    if (freq_atual <= freq_inverso)
                        inverte_mando(nova_populacao[i]);
                    // Se freq_atual > freq_inverso, protege (não muta)
                }
                else
                {
                    switch (mov)
                    {
                    case 0:
                        inverte_mando(nova_populacao[i]);
                        break;
                    case 1:
                        permuta_rodada(nova_populacao[i]);
                        break;
                    case 2:
                        permuta_times(nova_populacao[i]);
                        break;
                    }
                }
            }

            calcFO(nova_populacao[i]);
        }

        // Substituir não-elite pelos filhos (elite fica nas primeiras posições)
        memcpy(&populacao[num_elite], nova_populacao, sizeof(st_solucao) * num_filhos);

        // Ordenar por funObj
        sort_populacao(populacao, POP_SIZE);

        // Busca local nas top 3 elite
        int num_bl = MIN(3, num_elite);
        for (int i = 0; i < num_bl; i++)
        {
            if (usar_qvnd)
                qvnd(populacao[i]);
            else
                rvnd(populacao[i]);
            tempo = (double)(clock() - h_inicio) / CLOCKS_PER_SEC;
            if (tempo >= tempo_execucao)
                break;
        }

        // Decair epsilon do QVND ao longo das gerações
        if (usar_qvnd && q_epsilon > Q_EPSILON_MIN)
        {
            q_epsilon *= 0.999;
            if (q_epsilon < Q_EPSILON_MIN)
                q_epsilon = Q_EPSILON_MIN;
        }

        // Re-ordenar após busca local
        sort_populacao(populacao, POP_SIZE);

        // Atualizar melhor global
        if (populacao[0].funObj < s_best.funObj)
        {
            memcpy(&s_best, &populacao[0], sizeof(st_solucao));
            s_best.tempo_melhor_fo = (double)(clock() - h_inicio) / CLOCKS_PER_SEC;
            printf("GA Gen %d: melhor FO = %d TEMPO = %d\n", geracao, s_best.funObj, s_best.tempo_melhor_fo);
        }

        // Mineração: coleta elite (Fase 1) e transição (Fase 2)
        if (usar_mineracao)
        {
            if (!mineracao_ativa)
            {
                atualiza_pool_elite(populacao, num_elite);
                if (tempo >= tempo_execucao / 2.0)
                    minerar_padroes();
            }
        }

        // Diversidade: se elite convergiu, perturbar toda a população exceto o melhor
        if (populacao[0].funObj == populacao[num_elite - 1].funObj)
        {
            for (int i = 1; i < POP_SIZE; i++)
            {
                memcpy(&populacao[i], &populacao[0], sizeof(st_solucao));
                perturbarSolucao(PERTURBA_RODADA, PERTURBA_MANDO, PERTURBA_TIMES, populacao[i]);
                calcFO(populacao[i]);
            }
            sort_populacao(populacao, POP_SIZE);
        }

        // Log de estagnação a cada 200 gerações
        if (geracao % 200 == 0)
        {
            printf("Gen %d: best=%d, worst_elite=%d, best_filho=%d, crossover=%d/%d (%.0f%% ok)\n",
                   geracao, populacao[0].funObj, populacao[num_elite - 1].funObj,
                   populacao[num_elite].funObj,
                   crossover_ok, crossover_ok + crossover_fail,
                   (crossover_ok + crossover_fail > 0) ? 100.0 * crossover_ok / (crossover_ok + crossover_fail) : 0.0);
        }

        geracao++;
        tempo = (double)(clock() - h_inicio) / CLOCKS_PER_SEC;
    }

    printf("GA: %d geracoes, melhor FO = %d\n", geracao, s_best.funObj);
    printf("Crossover final: %d ok, %d fail (%.1f%% ok)\n",
           crossover_ok, crossover_fail,
           (crossover_ok + crossover_fail > 0) ? 100.0 * crossover_ok / (crossover_ok + crossover_fail) : 0.0);
    verifica_tabela_solucao(s_best);
}
