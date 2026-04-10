"""
Gera curvas de convergência (FO vs geração) para decidir MAX_GERACOES.
Roda N execuções independentes e plota média + min-max.
Uso: python scripts/convergencia_geracoes.py [max_ger] [num_exec] [metodo]
Rodar de dentro de src/
"""

import subprocess
import re
import sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

MAX_GER = int(sys.argv[1]) if len(sys.argv) > 1 else 500
NUM_EXEC = int(sys.argv[2]) if len(sys.argv) > 2 else 10
METODO = sys.argv[3] if len(sys.argv) > 3 else "ga"

INSTANCIAS = [
    ("masculina", "23-24"),
    ("feminina", "21-22"),
]

fig, axes = plt.subplots(1, len(INSTANCIAS), figsize=(7 * len(INSTANCIAS), 5))
if len(INSTANCIAS) == 1:
    axes = [axes]

for ax, (edicao, temp) in zip(axes, INSTANCIAS):
    print(f"\n=== {edicao} {temp} — {METODO} — {MAX_GER} gens × {NUM_EXEC} exec ===")

    all_curves = []

    for seed in range(1, NUM_EXEC + 1):
        cmd = f"./tcc2 {edicao} {temp} 1 {METODO} --max-geracoes {MAX_GER} --seed {seed}"
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        output = result.stdout

        # Parsear "GA Gen X: melhor FO = Y TEMPO = Z"
        gens = []
        fos = []
        best_fo = float('inf')

        # Capturar FO inicial
        init_match = re.search(r'GA Init: melhor FO = (\d+)', output)
        if init_match:
            best_fo = int(init_match.group(1))
            gens.append(0)
            fos.append(best_fo)

        for m in re.finditer(r'GA Gen (\d+): melhor FO = (\d+)', output):
            g = int(m.group(1))
            fo = int(m.group(2))
            if fo < best_fo:
                best_fo = fo
                gens.append(g)
                fos.append(fo)

        # Preencher curva completa (step function)
        curve = np.full(MAX_GER + 1, np.nan)
        if gens:
            curve[0] = fos[0]
            idx = 0
            for g in range(MAX_GER + 1):
                if idx + 1 < len(gens) and g >= gens[idx + 1]:
                    idx += 1
                curve[g] = fos[idx] if idx < len(fos) else fos[-1]

        all_curves.append(curve)
        final_match = re.search(r'melhor FO = (\d+)', output.split('\n')[-3] if len(output.split('\n')) > 2 else "")
        print(f"  seed {seed}: FO final = {best_fo}")

    all_curves = np.array(all_curves)
    media = np.nanmean(all_curves, axis=0)
    minimo = np.nanmin(all_curves, axis=0)
    maximo = np.nanmax(all_curves, axis=0)
    x = np.arange(MAX_GER + 1)

    ax.plot(x, media, color='#377eb8', linewidth=1.5, label='Média')
    ax.fill_between(x, minimo, maximo, color='#377eb8', alpha=0.15, label='Min-Max')
    ax.set_xlabel('Geração')
    ax.set_ylabel('Melhor FO')
    ax.set_title(f'{edicao.capitalize()} {temp} — {METODO.upper()}')
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    # Marcar gerações de referência
    for g_ref in [100, 150, 200, 300]:
        if g_ref <= MAX_GER:
            ax.axvline(x=g_ref, color='gray', linestyle='--', alpha=0.4)
            ax.text(g_ref + 2, ax.get_ylim()[1] * 0.99, f'g={g_ref}', fontsize=8, color='gray')

    # Imprimir valores em pontos-chave
    print(f"\n  Convergência ({edicao} {temp}):")
    for g_ref in [50, 100, 150, 200, 300, 500, 1000, 2000, 3000, 5000, 10000]:
        if g_ref <= MAX_GER:
            print(f"    Gen {g_ref:>5}: média={media[g_ref]:.0f}  min={minimo[g_ref]:.0f}  max={maximo[g_ref]:.0f}")

fig.suptitle(f'Convergência por geração — {METODO.upper()} ({NUM_EXEC} exec)', fontsize=14, fontweight='bold')
fig.tight_layout()

outfile = f'resultados-correcoes/convergencia_geracoes_{METODO}.png'
fig.savefig(outfile, dpi=150)
plt.close(fig)
print(f"\nSalvo: {outfile}")
