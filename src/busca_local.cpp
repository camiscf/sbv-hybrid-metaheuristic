#include "busca_local.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <ctime>

void BuscaLocal::movimento_troca_rodadas(Solucao& sol) {
    int turno = sol.n_rodadas / 2;
    int r1 = std::rand() % turno;
    int r2 = std::rand() % turno;
    while (r2 == r1) r2 = std::rand() % turno;

    for (int i = 0; i < sol.n_times; i++) {
        std::swap(sol.tabela[i][r1], sol.tabela[i][r2]);
        std::swap(sol.tabela[i][r1 + turno], sol.tabela[i][r2 + turno]);

        bool tmp = sol.casa[i][r1];
        sol.casa[i][r1] = sol.casa[i][r2];
        sol.casa[i][r2] = tmp;

        bool tmp2 = sol.casa[i][r1 + turno];
        sol.casa[i][r1 + turno] = sol.casa[i][r2 + turno];
        sol.casa[i][r2 + turno] = tmp2;
    }
}

void BuscaLocal::movimento_inversao_mando(Solucao& sol) {
    int turno = sol.n_rodadas / 2;
    int r = std::rand() % turno;
    int i = std::rand() % sol.n_times;
    int j = sol.tabela[i][r];

    bool tmp = sol.casa[i][r];
    sol.casa[i][r] = sol.casa[j][r];
    sol.casa[j][r] = tmp;

    bool tmp2 = sol.casa[i][r + turno];
    sol.casa[i][r + turno] = sol.casa[j][r + turno];
    sol.casa[j][r + turno] = tmp2;
}

void BuscaLocal::movimento_permuta_times(Solucao& sol) {
    int p = std::rand() % sol.n_times;
    int q = std::rand() % sol.n_times;
    while (q == p) q = std::rand() % sol.n_times;

    for (int r = 0; r < sol.n_rodadas; r++) {
        int adv_p = sol.tabela[p][r];
        int adv_q = sol.tabela[q][r];

        if (adv_p == q) {
            // p e q se enfrentam — só inverte mando
            bool tmp = sol.casa[p][r];
            sol.casa[p][r] = sol.casa[q][r];
            sol.casa[q][r] = tmp;
        } else {
            // Troca adversários
            sol.tabela[p][r]     = adv_q;
            sol.tabela[q][r]     = adv_p;
            sol.tabela[adv_p][r] = q;
            sol.tabela[adv_q][r] = p;

            // Troca mandos entre p e q
            bool tmp = sol.casa[p][r];
            sol.casa[p][r] = sol.casa[q][r];
            sol.casa[q][r] = tmp;
        }
    }
}

void BuscaLocal::perturbar_com_instancia(Solucao& sol, const Instancia& inst) {
    int turno = sol.n_rodadas / 2;
    int n_trocas      = std::max(1, (int)(sol.n_rodadas * rho));
    int n_inversoes   = std::max(1, (int)((sol.n_times / 2.0 * turno) * p_gamma));
    int n_permutacoes = std::max(1, (int)(sol.n_times * p_lambda));

    for (int k = 0; k < n_trocas;       k++) movimento_troca_rodadas(sol);
    for (int k = 0; k < n_inversoes;    k++) movimento_inversao_mando(sol);
    for (int k = 0; k < n_permutacoes;  k++) movimento_permuta_times(sol);

    sol.calcular_fo(inst);
}

Solucao BuscaLocal::gerar_vizinho(const Solucao& sol, const Instancia& inst) {
    Solucao vizinho = sol;
    int mov = std::rand() % 3;
    if      (mov == 0) movimento_troca_rodadas(vizinho);
    else if (mov == 1) movimento_inversao_mando(vizinho);
    else               movimento_permuta_times(vizinho);
    vizinho.calcular_fo(inst);
    return vizinho;
}

void BuscaLocal::sa(Solucao& sol, const Instancia& inst) {
    Solucao melhor = sol;
    Solucao atual  = sol;
    double T       = temperatura_inicial;

    while (T > temperatura_final) {
        for (int i = 0; i < n_vizinhos; i++) {
            Solucao viz  = gerar_vizinho(atual, inst);
            double delta = viz.fo - atual.fo;
            if (delta < 0) {
                atual = viz;
                if (atual.fo < melhor.fo) melhor = atual;
            } else {
                double prob = std::exp(-delta / T);
                if ((double)std::rand() / RAND_MAX < prob) atual = viz;
            }
        }
        T *= beta;
    }
    sol = melhor;
}

void BuscaLocal::ils(Solucao& sol, const Instancia& inst) {
    sa(sol, inst);
    Solucao melhor = sol;

    clock_t inicio = clock();
    int iter = 0;

    while (true) {
        double tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
        if (tempo >= tempo_limite) break;
        iter++;

        Solucao atual = melhor;
        perturbar_com_instancia(atual, inst);
        sa(atual, inst);

        if (atual.fo < melhor.fo) {
            melhor = atual;
            std::cout << "  iter " << iter
                      << " | " << (int)tempo << "s"
                      << " | FO = " << melhor.fo << std::endl;
        }
    }

    std::cout << "ILS: " << iter
              << " iteracoes | FO = " << melhor.fo << std::endl;
    sol = melhor;
}