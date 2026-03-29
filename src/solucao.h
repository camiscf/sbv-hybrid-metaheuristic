#pragma once
#include "instancia.h"
#include <vector>

// Número de rodadas = (n_times - 1) * 2
// Para 12 times = 22 rodadas

struct Solucao {
    int n_times;
    int n_rodadas;

    // tabela[i][r] = adversário do time i na rodada r
    std::vector<std::vector<int>> tabela;

    // casa[i][r] = true se time i joga em casa na rodada r
    std::vector<std::vector<bool>> casa;

    // Valor da função objetivo (distância total + penalidades)
    double fo;

    // Inicializa estrutura vazia
    void inicializar(int n);

    // Calcula a função objetivo completa
    double calcular_fo(const Instancia& inst);

    // Verifica se a solução é viável (sem violações)
    bool is_viavel() const;

    // Conta violações de cada restrição
    int violacoes_consecutivos_casa() const;
    int violacoes_consecutivos_fora() const;
    int violacoes_balanco_mando() const;

    // Imprime a tabela no terminal
    void imprimir() const;
    // Gera solução inicial pelo método do polígono
    void gerar_poligono(int seed = 42);
};