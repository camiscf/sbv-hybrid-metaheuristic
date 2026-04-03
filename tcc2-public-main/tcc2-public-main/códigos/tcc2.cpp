#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <math.h>
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

#define PASTA ".\\instancias"
#define CAMINHO_ARQUIVO_DADOS "\\dados-oficiais\\dados-oficiais-"
#define CAMINHO_ARQUIVO_RESULTADOS "\\resultados-calib\\calib-"
#define CAMINHO_ARQUIVO_TABELA_OFICIAL "\\tabela-oficial-"
#define TXT ".txt"
#define SOL ".sol"

#define EDICAO "\\feminina"
#define TEMPORADA "22-23"

int main(int argc, char *argv[])
{
    // if (argc < 7)
    // {
    //     // Exemplo de uso: .\tcc2.exe feminina-22-23 0.99 200 10 0.01 50
    //     printf("Uso: %s <instancia> <taxaResfriamento> <numeroIteracoes> <temperaturaInicial> <temperaturaFinal> <maxExec> <pertb_rodada> <pertb_mando> <pertb_times>\n", argv[0]);
    //     return 1;
    // }

    // const char *instancia = argv[1];
    // TAXA_RESFRIAMENTO = atof(argv[2]);
    // NUMERO_ITERACOES = atoi(argv[3]);
    // TEMPERATURA_INICIAL = atoi(argv[4]);
    // PERTURBA_RODADA = atof(argv[5]);
    // PERTURBA_MANDO = atof(argv[6]);
    // PERTURBA_TIMES = atof(argv[7]);

    srand(time(0));

    // ##### TROCAR PARA EXECUTAR IRACE #####

    // IRACE
    // char caminho_irace[100];
    // snprintf(caminho_irace, sizeof(caminho_irace), "./instancias/%s.txt", instancia);
    // ler_instancia(caminho_irace);

    // EXECUCAO NORMAL

    // const char *caminho = PASTA EDICAO CAMINHO_ARQUIVO_DADOS TEMPORADA TXT;
    // printf("Caminho do arquivo: %s\n", caminho);
    // ler_instancia(caminho);

    // ##### FIM DA TROCA  #####

    // EXECUÇÕES DE TESTE

    const char *ed[] = {"masculina", "feminina"};
    const char *temp[] = {"22-23", "23-24", "24-25"};
    char caminho_teste[100];
    int cabecalho = 1;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            snprintf(caminho_teste, sizeof(caminho_teste), ".\\instancias\\%s%s%s.txt", ed[i], CAMINHO_ARQUIVO_DADOS, temp[j]);
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

    // escreve_instancia();

    // executar_leitura_tabela_resultado();

    // st_solucao s;
    // iteratedLocalSearch(s);

    // escreve_resultado_solucao(s);

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

    int time_sorteado[num_times - 1];
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

    // calcFO(s);
    printf("\nFO = %d\nDist Total = %d\nTempo Melhor FO = %d\n", s.funObj, s.total_dist, s.tempo_melhor_fo);
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
