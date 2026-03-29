#include <iostream>
#include "instancia.h"
#include "solucao.h"
#include "busca_local.h"

int main() {
    std::srand(42);

    Instancia inst;
    if (!inst.carregar("../data/instancia_M2425.txt")) return 1;

    // Gera solução inicial
    Solucao sol;
    sol.inicializar(inst.n_times);
    sol.gerar_poligono(42);
    sol.calcular_fo(inst);

    std::cout << "Antes do SA:" << std::endl;
    std::cout << "  FO     = " << sol.fo << std::endl;
    std::cout << "  Viável = " << (sol.is_viavel() ? "sim" : "não") << std::endl;

    // Aplica SA
    BuscaLocal bl;
    bl.sa(sol, inst);

    std::cout << "\nDepois do SA:" << std::endl;
    std::cout << "  FO     = " << sol.fo << std::endl;
    std::cout << "  Viável = " << (sol.is_viavel() ? "sim" : "não") << std::endl;

    return 0;
}