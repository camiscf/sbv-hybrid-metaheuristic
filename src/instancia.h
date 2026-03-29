#pragma once
#include <vector>
#include <string>

struct Instancia {
    int n_times;
    std::vector<std::string> nomes;
    std::vector<std::vector<double>> distancias;

    bool carregar(const std::string& caminho);

    double distancia(int i, int j) const {
        return distancias[i][j];
    }
};