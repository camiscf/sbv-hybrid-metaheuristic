#include "ga.h"
#include <algorithm>
#include <iostream>
#include <ctime>
#include <cstdlib>

// ── Inicializa população ──────────────────────────────────────────────
std::vector<Solucao> GA::inicializar_populacao(const Instancia& inst) {
    std::vector<Solucao> pop(tamanho_populacao);
    for (int i = 0; i < tamanho_populacao; i++) {
        pop[i].inicializar(inst.n_times);
        pop[i].gerar_poligono(i); // seed diferente por indivíduo
        pop[i].calcular_fo(inst);
    }
    return pop;
}

// ── Seleção por torneio (k=3) ─────────────────────────────────────────
Solucao& GA::selecionar(std::vector<Solucao>& pop) {
    int k = 3;
    int melhor_idx = std::rand() % pop.size();
    for (int i = 1; i < k; i++) {
        int idx = std::rand() % pop.size();
        if (pop[idx].fo < pop[melhor_idx].fo)
            melhor_idx = idx;
    }
    return pop[melhor_idx];
}

// ── Crossover por turno/returno ───────────────────────────────────────
Solucao GA::crossover(const Solucao& pai1, const Solucao& pai2,
                      const Instancia& inst) {
    Solucao filho;
    filho.inicializar(inst.n_times);

    int turno = filho.n_rodadas / 2;

    // Herda turno do pai1 e returno do pai2
    for (int i = 0; i < filho.n_times; i++) {
        for (int r = 0; r < turno; r++) {
            filho.tabela[i][r] = pai1.tabela[i][r];
            filho.casa[i][r]   = pai1.casa[i][r];
        }
    }

    // Espelha o turno para gerar o returno
    for (int i = 0; i < filho.n_times; i++) {
        for (int r = 0; r < turno; r++) {
            int r_ret = r + turno;
            filho.tabela[i][r_ret] = filho.tabela[i][r];
            filho.casa[i][r_ret]   = !filho.casa[i][r]; // mando invertido
        }
    }

    filho.calcular_fo(inst);
    return filho;
}

// ── Mutação ───────────────────────────────────────────────────────────
void GA::mutar(Solucao& sol, const Instancia& inst) {
    if ((double)std::rand() / RAND_MAX > prob_mutacao) return;

    int mov = std::rand() % 3;
    if      (mov == 0) bl.movimento_troca_rodadas(sol);
    else if (mov == 1) bl.movimento_inversao_mando(sol);
    else               bl.movimento_permuta_times(sol);

    sol.calcular_fo(inst);
}

// ── Aplica SA nas elite ───────────────────────────────────────────────
void GA::aplicar_busca_local_elite(std::vector<Solucao>& pop,
                                    const Instancia& inst) {
    // Ordena por FO (menor = melhor)
    std::sort(pop.begin(), pop.end(),
              [](const Solucao& a, const Solucao& b) {
                  return a.fo < b.fo;
              });

    // Aplica SA nos elite (30% melhores)
    int n_elite = (int)(pop.size() * fracao_elite);
    for (int i = 0; i < n_elite; i++) {
        bl.sa(pop[i], inst);
    }
}

// ── GA principal ──────────────────────────────────────────────────────
void GA::executar(Solucao& melhor_global, const Instancia& inst) {
    // Inicializa população
    std::vector<Solucao> pop = inicializar_populacao(inst);

    // Aplica SA em todos na geração 0
    aplicar_busca_local_elite(pop, inst);

    // Melhor da população inicial
    melhor_global = pop[0];
    std::cout << "Geração 0 | FO = " << melhor_global.fo << std::endl;

    clock_t inicio = clock();
    int geracao = 0;

    while (true) {
        double tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
        if (tempo >= tempo_limite) break;
        geracao++;

        // Gera nova população por crossover
        std::vector<Solucao> nova_pop;
        nova_pop.reserve(tamanho_populacao);

        while ((int)nova_pop.size() < tamanho_populacao) {
            Solucao& pai1 = selecionar(pop);
            Solucao& pai2 = selecionar(pop);

            Solucao filho = crossover(pai1, pai2, inst);
            mutar(filho, inst);
            nova_pop.push_back(filho);
        }

        // Substitui população
        pop = nova_pop;

        // Aplica SA nas elite
        aplicar_busca_local_elite(pop, inst);

        // Atualiza melhor global
        if (pop[0].fo < melhor_global.fo) {
            melhor_global = pop[0];
            std::cout << "Geração " << geracao
                      << " | " << (int)tempo << "s"
                      << " | FO = " << melhor_global.fo << std::endl;
        }
    }

    std::cout << "GA: " << geracao
              << " gerações | FO = " << melhor_global.fo << std::endl;
}