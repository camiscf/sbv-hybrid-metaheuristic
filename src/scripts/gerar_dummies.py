"""
Gera 30 instâncias dummy estatísticas para pré-treino do Q-Learning.
Amostra distâncias de N(μ, σ) das 8 instâncias reais.
Garante simetria e desigualdade triangular (Floyd-Warshall).
Formato idêntico às instâncias reais.
"""

import os
import numpy as np

INSTANCIAS_DIR = "../instancias"
DUMMIES_DIR = "dummies"
NUM_DUMMIES = 30
N_TIMES = 12
SEED = 42

def ler_instancia(path):
    """Lê uma instância e retorna a matriz de distâncias e (max_home, max_away, dif)."""
    with open(path) as f:
        n = int(f.readline().strip())
        nomes = [f.readline().strip() for _ in range(n)]
        mat = []
        for _ in range(n):
            mat.append(list(map(int, f.readline().split())))
        params = [int(f.readline().strip()) for _ in range(3)]
    return n, np.array(mat), params

def corrigir_desigualdade_triangular(mat):
    """Floyd-Warshall para garantir desigualdade triangular."""
    n = len(mat)
    d = mat.copy().astype(float)
    for k in range(n):
        for i in range(n):
            for j in range(n):
                if i != j and d[i][k] + d[k][j] < d[i][j]:
                    d[i][j] = d[i][k] + d[k][j]
    return np.round(d).astype(int)

def main():
    np.random.seed(SEED)
    os.makedirs(DUMMIES_DIR, exist_ok=True)

    # Coletar todas as distâncias não-diagonais das 8 instâncias reais
    todas_dist = []
    todos_params = []

    for genero in ["masculina", "feminina"]:
        for temp in ["21-22", "22-23", "23-24", "24-25"]:
            path = os.path.join(INSTANCIAS_DIR, genero, f"dados-oficiais-{temp}.txt")
            n, mat, params = ler_instancia(path)
            todos_params.append(params)
            for i in range(n):
                for j in range(n):
                    if i != j:
                        todas_dist.append(mat[i][j])

    todas_dist = np.array(todas_dist)
    media = todas_dist.mean()
    desvio = todas_dist.std()
    print(f"Estatísticas das 8 instâncias reais: μ={media:.1f}, σ={desvio:.1f}")
    print(f"Min={todas_dist.min()}, Max={todas_dist.max()}")

    for d in range(1, NUM_DUMMIES + 1):
        # Gerar matriz simétrica
        mat = np.zeros((N_TIMES, N_TIMES), dtype=int)
        for i in range(N_TIMES):
            for j in range(i + 1, N_TIMES):
                val = max(100, int(np.random.normal(media, desvio)))
                mat[i][j] = val
                mat[j][i] = val

        # Corrigir desigualdade triangular
        mat = corrigir_desigualdade_triangular(mat)
        # Garantir simetria pós-Floyd (arredondamento pode gerar assimetria residual)
        for i in range(N_TIMES):
            mat[i][i] = 0
            for j in range(i + 1, N_TIMES):
                val = min(mat[i][j], mat[j][i])
                mat[i][j] = val
                mat[j][i] = val

        # Copiar max_home, max_away, dif de uma instância real aleatória
        params = todos_params[np.random.randint(len(todos_params))]

        # Salvar
        path = os.path.join(DUMMIES_DIR, f"dummy-{d:02d}.txt")
        with open(path, "w") as f:
            f.write(f"{N_TIMES}\n")
            for i in range(1, N_TIMES + 1):
                f.write(f"Time_{i:02d}\n")
            for i in range(N_TIMES):
                f.write("\t".join(str(mat[i][j]) for j in range(N_TIMES)) + "\n")
            for p in params:
                f.write(f"{p}\n")

        print(f"  {path} gerado")

    print(f"\n{NUM_DUMMIES} instâncias dummy salvas em {DUMMIES_DIR}/")

if __name__ == "__main__":
    main()
