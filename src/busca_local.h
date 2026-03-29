#pragma once
#include "solucao.h"
#include "instancia.h"

struct BuscaLocal {

    // Parâmetros SA
    double temperatura_inicial = 100.0;
    double temperatura_final   = 0.01;
    double beta                = 0.9362;
    int    n_vizinhos          = 500;

    // Parâmetros perturbação ILS
    double rho      = 0.3802;
    double p_gamma  = 0.8537;
    double p_lambda = 0.3939;

    // Tempo limite
    double tempo_limite = 300.0;

    // SA
    void sa(Solucao& sol, const Instancia& inst);

    // ILS
    void ils(Solucao& sol, const Instancia& inst);

    // Perturbação
    void perturbar_com_instancia(Solucao& sol, const Instancia& inst);

    // Movimentos
    void movimento_troca_rodadas(Solucao& sol);
    void movimento_inversao_mando(Solucao& sol);
    void movimento_permuta_times(Solucao& sol);

    // Gera vizinho
    Solucao gerar_vizinho(const Solucao& sol, const Instancia& inst);
};