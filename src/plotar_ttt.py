"""
Gera TTTPlots visuais (distribuição empírica de probabilidade acumulada).
Para 3 instâncias representativas, gera figura com 3 subplots (1 por checkpoint).
"""

import os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd

TTT_DIR = "resultados-correcoes/ttt"
ALVOS_CSV = "resultados-correcoes/alvos.csv"
FIGDIR = "resultados-correcoes/figuras"
os.makedirs(FIGDIR, exist_ok=True)

METODOS = ["ils", "ga", "qvnd", "gam", "qvndm"]
LABELS = {"ils": "ILS+SA", "ga": "GA+RVND", "qvnd": "GA+QVND", "gam": "GA+RVND+Min", "qvndm": "GA+QVND+Min"}
CORES = {"ils": "#e41a1c", "ga": "#377eb8", "qvnd": "#4daf4a", "gam": "#984ea3", "qvndm": "#ff7f00"}
CHECKPOINTS = ["0%", "-2%", "-5%"]

# Instâncias representativas (ajustar após resultados)
INSTANCIAS_REPR = [
    ("masculina", "23-24"),
    ("masculina", "24-25"),
    ("feminina", "22-23"),
]

alvos_df = pd.read_csv(ALVOS_CSV)

def coletar_tempos(metodo, edicao, temp, checkpoint_idx):
    """Coleta todos os tempos de atingimento para um checkpoint."""
    tempos = []
    for exec_num in range(1, 100):
        fname = f"{metodo}_{edicao}_{temp}_exec{exec_num}.txt"
        path = os.path.join(TTT_DIR, fname)
        if not os.path.exists(path):
            if exec_num > 1:
                break
            continue
        with open(path) as f:
            lines = f.readlines()
            if checkpoint_idx < len(lines):
                parts = lines[checkpoint_idx].strip().split()
                if len(parts) >= 2:
                    t = float(parts[1])
                    if t >= 0:
                        tempos.append(t)
    return tempos

for edicao, temp in INSTANCIAS_REPR:
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    inst_label = f"{edicao.capitalize()} {temp}"

    for cp_idx, (ax, cp_label) in enumerate(zip(axes, CHECKPOINTS)):
        for metodo in METODOS:
            tempos = coletar_tempos(metodo, edicao, temp, cp_idx)
            if not tempos:
                continue

            # Distribuição empírica acumulada
            tempos_sorted = np.sort(tempos)
            n = len(tempos_sorted)
            prob = np.arange(1, n + 1) / n  # P(T <= t)

            ax.step(tempos_sorted, prob, where='post',
                    label=LABELS[metodo], color=CORES[metodo], linewidth=1.5)

        # Buscar valor do alvo
        alvo_row = alvos_df[(alvos_df["edicao"] == edicao) & (alvos_df["temporada"] == temp)]
        if not alvo_row.empty:
            alvo_cols = ["alvo_0pct", "alvo_2pct", "alvo_5pct"]
            alvo_val = alvo_row.iloc[0][alvo_cols[cp_idx]]
            ax.set_title(f"Checkpoint {cp_label} (alvo={alvo_val:,.0f})")
        else:
            ax.set_title(f"Checkpoint {cp_label}")

        ax.set_xlabel("Tempo (s)")
        ax.set_ylabel("P(atingir alvo)")
        ax.set_ylim(-0.05, 1.05)
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=8)

    fig.suptitle(f"TTTPlot — {inst_label}", fontsize=14, fontweight='bold')
    fig.tight_layout()

    fname = f"tttplot_{edicao}_{temp}.png"
    fig.savefig(os.path.join(FIGDIR, fname), dpi=150)
    plt.close(fig)
    print(f"Salvo: {FIGDIR}/{fname}")

print("\nTTTPlots gerados.")
