#include "solucao.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>

void Solucao::inicializar(int n) {
    n_times   = n;
    n_rodadas = (n - 1) * 2;
    fo        = 1e18; // valor alto — solução não avaliada

    tabela.assign(n_times, std::vector<int>(n_rodadas, -1));
    casa.assign(n_times, std::vector<bool>(n_rodadas, false));
}

double Solucao::calcular_fo(const Instancia& inst) {
    double distancia_total = 0.0;
    double penalidade      = 1000000.0;

    for (int i = 0; i < n_times; i++) {
        int local_atual = i; // começa na cidade sede do time i

        for (int r = 0; r < n_rodadas; r++) {
            int adversario = tabela[i][r];

            if (casa[i][r]) {
                // Joga em casa — se estava fora, viaja de volta
                if (local_atual != i) {
                    distancia_total += inst.distancia(local_atual, i);
                    local_atual = i;
                }
            } else {
                // Joga fora — viaja da localização atual até a cidade do adversário
                distancia_total += inst.distancia(local_atual, adversario);
                local_atual = adversario;
            }
        }

        // Volta para casa ao final do campeonato
        if (local_atual != i) {
            distancia_total += inst.distancia(local_atual, i);
        }
    }

    // Penalidades por violações de restrição
    distancia_total += violacoes_consecutivos_casa() * penalidade;
    distancia_total += violacoes_consecutivos_fora() * penalidade;
    distancia_total += violacoes_balanco_mando()     * penalidade;

    fo = distancia_total;
    return fo;
}

bool Solucao::is_viavel() const {
    return violacoes_consecutivos_casa() == 0 &&
           violacoes_consecutivos_fora() == 0 &&
           violacoes_balanco_mando()     == 0;
}

int Solucao::violacoes_consecutivos_casa() const {
    int violacoes = 0;
    for (int i = 0; i < n_times; i++) {
        int consecutivos = 0;
        for (int r = 0; r < n_rodadas; r++) {
            if (casa[i][r]) {
                consecutivos++;
                if (consecutivos > 6) violacoes++;
            } else {
                consecutivos = 0;
            }
        }
    }
    return violacoes;
}

int Solucao::violacoes_consecutivos_fora() const {
    int violacoes = 0;
    for (int i = 0; i < n_times; i++) {
        int consecutivos = 0;
        for (int r = 0; r < n_rodadas; r++) {
            if (!casa[i][r]) {
                consecutivos++;
                if (consecutivos > 6) violacoes++;
            } else {
                consecutivos = 0;
            }
        }
    }
    return violacoes;
}

int Solucao::violacoes_balanco_mando() const {
    // Diferença entre jogos em casa e fora no turno
    // não pode ser maior que 6
    int violacoes = 0;
    int turno = n_rodadas / 2;

    for (int i = 0; i < n_times; i++) {
        int em_casa = 0, fora = 0;
        for (int r = 0; r < turno; r++) {
            if (casa[i][r]) em_casa++;
            else fora++;
        }
        if (std::abs(em_casa - fora) > 6) violacoes++;
    }
    return violacoes;
}

void Solucao::imprimir() const {
    std::cout << "FO = " << fo << std::endl;
    std::cout << "Viável: " << (is_viavel() ? "sim" : "não") << std::endl;
    for (int r = 0; r < n_rodadas; r++) {
        std::cout << "Rodada " << r + 1 << ": ";
        for (int i = 0; i < n_times; i++) {
            if (casa[i][r] && tabela[i][r] > i) {
                std::cout << i << "(casa) x " << tabela[i][r] << "  ";
            }
        }
        std::cout << std::endl;
    }
}

void Solucao::gerar_poligono(int seed) {
    // Método do polígono para gerar tabela espelhada
    // Baseado em Biajoli [2007] conforme artigo base

    std::srand(seed);
    int n = n_times;
    int turno = n_rodadas / 2; // 11 rodadas

    // Cria vetor de times e embaralha
    std::vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = i;

    // Escolhe time base aleatoriamente e coloca na posição 0
    int base_idx = std::rand() % n;
    std::swap(v[0], v[base_idx]);

    // Embaralha o restante (posições 1 a n-1)
    for (int i = n - 1; i > 1; i--) {
        int j = 1 + std::rand() % i;
        std::swap(v[i], v[j]);
    }

    // Gera as rodadas do turno
    for (int r = 0; r < turno; r++) {
        // Time base (v[0]) joga contra v[1]
        // Em rodadas ímpares: base joga em casa
        // Em rodadas pares: base joga fora
        int t1 = v[0], t2 = v[1];
        bool base_casa = (r % 2 == 0);

        tabela[t1][r] = t2;
        tabela[t2][r] = t1;
        casa[t1][r]   = base_casa;
        casa[t2][r]   = !base_casa;

        // Demais confrontos: posição i vs posição n-i+1
        for (int i = 2; i <= n / 2; i++) {
            int a = v[i];
            int b = v[n - i + 1];

            tabela[a][r] = b;
            tabela[b][r] = a;

            // Time na posição ímpar joga em casa
            bool a_casa = (i % 2 != 0);
            casa[a][r]  = a_casa;
            casa[b][r]  = !a_casa;
        }

        // Rotaciona v[1..n-1] no sentido horário
        // v[2..n] → v[1..n-1], v[1] → v[n]
        int ultimo = v[1];
        for (int i = 1; i < n - 1; i++) v[i] = v[i + 1];
        v[n - 1] = ultimo;
    }

    // Gera returno espelhando o turno (mando invertido)
    for (int r = 0; r < turno; r++) {
        for (int i = 0; i < n; i++) {
            int r_returno = r + turno;
            tabela[i][r_returno] = tabela[i][r];
            casa[i][r_returno]   = !casa[i][r]; // inverte mando
        }
    }
}