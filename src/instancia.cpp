#include "instancia.h"
#include <fstream>
#include <iostream>

bool Instancia::carregar(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir: " << caminho << std::endl;
        return false;
    }

    arquivo >> n_times;

    nomes.resize(n_times);
    for (int i = 0; i < n_times; i++) {
        arquivo >> nomes[i];
    }

    distancias.resize(n_times, std::vector<double>(n_times));
    for (int i = 0; i < n_times; i++) {
        for (int j = 0; j < n_times; j++) {
            arquivo >> distancias[i][j];
        }
    }

    arquivo.close();
    return true;
}