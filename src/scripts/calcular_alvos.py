"""
Calcula os 3 alvos TTT por instância a partir dos resultados do ILS.
Checkpoint 1 (0%):  fbest do ILS
Checkpoint 2 (-2%): fbest do ILS × 0.98
Checkpoint 3 (-5%): fbest do ILS × 0.95
Saída: alvos.csv
"""

import pandas as pd
import os

# Usar os resultados do ILS com 600s (mais dados) se disponíveis
ILS_CSV = "resultados-600s/resultados-ils.csv"
if not os.path.exists(ILS_CSV):
    ILS_CSV = "resultados-ils.csv"

df = pd.read_csv(ILS_CSV, sep=";")

rows = []
for (edicao, temp), g in df.groupby(["EDICAO", "TEMPORADA"]):
    fbest = g["FO"].min()
    alvos = [
        int(fbest),                # 0%: igualar o melhor do ILS
        int(fbest * 0.98),         # -2%: superar em 2%
        int(fbest * 0.95),         # -5%: superar em 5%
    ]
    rows.append({
        "edicao": edicao,
        "temporada": temp,
        "alvo_0pct": alvos[0],
        "alvo_2pct": alvos[1],
        "alvo_5pct": alvos[2],
    })
    print(f"{edicao} {temp}: fbest={fbest} → alvos={alvos}")

out = pd.DataFrame(rows)
out.to_csv("resultados-correcoes/alvos.csv", index=False)
print(f"\nSalvo em resultados-correcoes/alvos.csv ({len(rows)} instâncias)")
