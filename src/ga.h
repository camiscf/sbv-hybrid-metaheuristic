#pragma once
#include "solucao.h"
#include "instancia.h"
#include "busca_local.h"
#include <vector>

struct GA {

    // Parâmetros
    int    tamanho_populacao = 50;
    double fracao_elite      = 0.30; // 30% elite
    double prob_mutacao      = 0.20;
    double tempo_limite      = 300.0;

    // Busca local (SA para não-elite, RVND para elite — por ora SA para ambos)
    BuscaLocal bl;

    // Executa o GA
    void executar(Solucao& melhor, const Instancia& inst);

    // Inicializa população com método do polígono
    std::vector<Solucao> inicializar_populacao(const Instancia& inst);

    // Seleção por torneio (k=3)
    Solucao& selecionar(std::vector<Solucao>& pop);

    // Crossover por turno/returno
    Solucao crossover(const Solucao& pai1, const Solucao& pai2,
                      const Instancia& inst);

    // Mutação: aplica um movimento aleatório
    void mutar(Solucao& sol, const Instancia& inst);

    // Divide e aplica busca local nas elite
    void aplicar_busca_local_elite(std::vector<Solucao>& pop,
                                   const Instancia& inst);
};