"""
Agrega resultados TTT (Time-To-Target) dos experimentos.
Lê arquivos em resultados-correcoes/ttt/ e gera:
  - resultados-correcoes/tabela_ttt.csv
  - resultados-correcoes/tabela_ttt_formatada.md
"""

import os
import pandas as pd
import numpy as np

TTT_DIR = "resultados-correcoes/ttt"
ALVOS_CSV = "resultados-correcoes/alvos.csv"
METODOS = ["ils", "ga", "qvnd", "gam", "qvndm"]
LABELS = {"ils": "ILS+SA", "ga": "GA+RVND", "qvnd": "GA+QVND", "gam": "GA+RVND+Min", "qvndm": "GA+QVND+Min"}
CHECKPOINTS = ["0%", "-2%", "-5%"]

# Ler alvos
alvos_df = pd.read_csv(ALVOS_CSV)

rows = []
for _, alvo_row in alvos_df.iterrows():
    edicao = alvo_row["edicao"]
    temp = alvo_row["temporada"]
    inst = f"{edicao} {temp}"

    for metodo in METODOS:
        tempos_por_checkpoint = [[], [], []]

        # Ler todos os arquivos TTT desse método/instância
        for exec_num in range(1, 100):  # procurar até 99
            fname = f"{metodo}_{edicao}_{temp}_exec{exec_num}.txt"
            path = os.path.join(TTT_DIR, fname)
            if not os.path.exists(path):
                if exec_num > 1:
                    break
                continue

            with open(path) as f:
                for i, line in enumerate(f):
                    if i >= 3:
                        break
                    parts = line.strip().split()
                    if len(parts) >= 2:
                        tempo = float(parts[1])
                        tempos_por_checkpoint[i].append(tempo)

        n_exec = len(tempos_por_checkpoint[0])
        if n_exec == 0:
            continue

        for cp_idx, cp_label in enumerate(CHECKPOINTS):
            tempos = tempos_por_checkpoint[cp_idx]
            atingiu = [t for t in tempos if t >= 0]
            nao_atingiu = len(tempos) - len(atingiu)

            mediana = np.median(atingiu) if atingiu else -1
            taxa = len(atingiu) / len(tempos) * 100 if tempos else 0

            rows.append({
                "instancia": inst,
                "metodo": LABELS[metodo],
                "checkpoint": cp_label,
                "mediana_s": round(mediana, 3) if mediana >= 0 else -1,
                "taxa_sucesso_pct": round(taxa, 1),
                "n_atingiu": len(atingiu),
                "n_total": len(tempos),
            })

df = pd.DataFrame(rows)
df.to_csv("resultados-correcoes/tabela_ttt.csv", index=False)
print(f"Salvo: resultados-correcoes/tabela_ttt.csv ({len(df)} linhas)")

# Gerar tabela markdown formatada
with open("resultados-correcoes/tabela_ttt_formatada.md", "w") as f:
    f.write("# Tabela TTT — Time-To-Target\n\n")
    f.write("Mediana do tempo (s) para atingir cada checkpoint. Taxa de sucesso entre parênteses.\n\n")

    for cp_label in CHECKPOINTS:
        cp_df = df[df["checkpoint"] == cp_label]
        f.write(f"## Checkpoint {cp_label}\n\n")
        f.write(f"| Instância | " + " | ".join(LABELS[m] for m in METODOS) + " |\n")
        f.write(f"|-----------|" + "|".join(["--------" for _ in METODOS]) + "|\n")

        for inst in cp_df["instancia"].unique():
            inst_df = cp_df[cp_df["instancia"] == inst]
            cells = []
            for metodo in METODOS:
                row = inst_df[inst_df["metodo"] == LABELS[metodo]]
                if row.empty:
                    cells.append("--")
                else:
                    r = row.iloc[0]
                    if r["mediana_s"] < 0:
                        cells.append(f"-- (0/{r['n_total']})")
                    else:
                        cells.append(f"{r['mediana_s']:.1f}s ({r['n_atingiu']}/{r['n_total']})")
            f.write(f"| {inst} | " + " | ".join(cells) + " |\n")
        f.write("\n")

print("Salvo: resultados-correcoes/tabela_ttt_formatada.md")
