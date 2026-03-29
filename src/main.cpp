#include <iostream>
#include "instancia.h"
#include "solucao.h"
#include "ga.h"

int main() {
    std::srand(42);

    Instancia inst;
    if (!inst.carregar("../data/instancia_M2425.txt")) return 1;

    std::cout << "Iniciando GA..." << std::endl;

    GA ga;
    ga.tempo_limite = 30.0; // 30 segundos para teste

    Solucao melhor;
    melhor.inicializar(inst.n_times);
    ga.executar(melhor, inst);

    std::cout << "\nFinal: FO = " << melhor.fo
              << " | Viável = " << (melhor.is_viavel() ? "sim" : "não")
              << std::endl;

    return 0;
}