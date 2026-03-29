#include "busca_local.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

// ── Movimento 1 — troca de ordem entre duas rodadas ──────────────────
void BuscaLocal::movimento_troca_rodadas(Solucao& sol) {
    int turno = sol.n_rodadas / 2;

    // Sorteia duas rodadas distintas do turno
    int r1 = std::rand() % turno;
    int r2 = std::rand() % turno;
    while (r2 == r1) r2 = std::rand() % turno;

    int r1_ret = r1 + turno;
    int r2_ret = r2 + turno;

    // Troca no turno e no returno
    for (int i = 0; i < sol.n_times; i++) {
        std::swap(sol.tabela[i][r1],     sol.tabela[i][r2]);
        std::swap(sol.casa[i][r1],       sol.casa[i][r2]);
        std::swap(sol.tabela[i][r1_ret], sol.tabela[i][r2_ret]);
        std::swap(sol.casa[i][r1_ret],   sol.casa[i][r2_ret]);
    }
}

// ── Movimento 2 — inversão do mando de campo ─────────────────────────
void BuscaLocal::movimento_inversao_mando(Solucao& sol) {
    int turno = sol.n_rodadas / 2;

    // Sorteia uma rodada do turno e um time
    int r = std::rand() % turno;
    int i = std::rand() % sol.n_times;
    int j = sol.tabela[i][r]; // adversário

    int r_ret = r + turno;

    // Inverte mando no turno
    sol.casa[i][r] = !sol.casa[i][r];
    sol.casa[j][r] = !sol.casa[j][r];

    // Inverte mando no returno (espelho)
    sol.casa[i][r_ret] = !sol.casa[i][r_ret];
    sol.casa[j][r_ret] = !sol.casa[j][r_ret];
}

// ── Movimento 3 — permutação de dois times ───────────────────────────
void BuscaLocal::movimento_permuta_times(Solucao& sol) {
    // Sorteia dois times distintos
    int p = std::rand() % sol.n_times;
    int q = std::rand() % sol.n_times;
    while (q == p) q = std::rand() % sol.n_times;

    for (int r = 0; r < sol.n_rodadas; r++) {
        int adv_p = sol.tabela[p][r];
        int adv_q = sol.tabela[q][r];

        if (adv_p == q) {
            // p e q se enfrentam — só inverte mando
            sol.casa[p][r] = !sol.casa[p][r];
            sol.casa[q][r] = !sol.casa[q][r];
        } else {
            // Troca adversários
            sol.tabela[p][r] = adv_q;
            sol.tabela[q][r] = adv_p;

            // Atualiza referências dos adversários
            sol.tabela[adv_p][r] = q;
            sol.tabela[adv_q][r] = p;

            // Troca mandos entre p e q
            bool casa_p = sol.casa[p][r];
            sol.casa[p][r] = sol.casa[q][r];
            sol.casa[q][r] = casa_p;
        }
    }
}

// ── Gera vizinho sorteando um movimento ──────────────────────────────
Solucao BuscaLocal::gerar_vizinho(const Solucao& sol, const Instancia& inst) {
    Solucao vizinho = sol;

    int movimento = std::rand() % 3;
    if      (movimento == 0) movimento_troca_rodadas(vizinho);
    else if (movimento == 1) movimento_inversao_mando(vizinho);
    else                     movimento_permuta_times(vizinho);

    vizinho.calcular_fo(inst);
    return vizinho;
}

// ── SA principal ─────────────────────────────────────────────────────
void BuscaLocal::sa(Solucao& sol, const Instancia& inst) {
    Solucao melhor = sol;
    Solucao atual  = sol;
    double T       = temperatura_inicial;

    while (T > temperatura_final) {
        for (int i = 0; i < n_vizinhos; i++) {
            Solucao vizinho = gerar_vizinho(atual, inst);
            double delta    = vizinho.fo - atual.fo;

            if (delta < 0) {
                atual = vizinho;
                if (atual.fo < melhor.fo) melhor = atual;
            } else {
                double prob = std::exp(-delta / T);
                double r    = (double)std::rand() / RAND_MAX;
                if (r < prob) atual = vizinho;
            }
        }
        T *= beta;
    }

    sol = melhor;
}