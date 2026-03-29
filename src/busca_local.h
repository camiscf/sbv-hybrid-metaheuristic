#pragma once
#include "solucao.h"
#include "instancia.h"

struct BuscaLocal {

    // Parâmetros do SA (Tabela 1 do artigo base)
    double temperatura_inicial = 100.0;
    double temperatura_final   = 0.01;
    double beta                = 0.9362;
    int    n_vizinhos          = 500;

    // Executa SA sobre uma solução
    void sa(Solucao& sol, const Instancia& inst);

    // Os três movimentos de vizinhança
    void movimento_troca_rodadas(Solucao& sol);
    void movimento_inversao_mando(Solucao& sol);
    void movimento_permuta_times(Solucao& sol);

    // Gera um vizinho sorteando um dos três movimentos
    Solucao gerar_vizinho(const Solucao& sol, const Instancia& inst);
};