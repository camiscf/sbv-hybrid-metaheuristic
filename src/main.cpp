#include <iostream>
#include "instancia.h"
#include "solucao.h"
#include "busca_local.h"

int main() {
    std::srand(42);

    Instancia inst;
    if (!inst.carregar("../data/instancia_M2425.txt")) return 1;

    Solucao sol;
    sol.inicializar(inst.n_times);
    sol.gerar_poligono(42);
    sol.calcular_fo(inst);

    std::cout << "Solução inicial: FO = " << sol.fo << std::endl;

    BuscaLocal bl;
    bl.tempo_limite = 30.0;
    bl.ils(sol, inst);

    std::cout << "\nFinal: FO = " << sol.fo
              << " | Viável = " << (sol.is_viavel() ? "sim" : "não")
              << std::endl;

    return 0;
}