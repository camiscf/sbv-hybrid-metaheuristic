import os
import sys
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# Pasta de dados: argumento ou src/
if len(sys.argv) > 1:
    DATADIR = sys.argv[1]
else:
    DATADIR = os.path.dirname(os.path.abspath(__file__))

METODOS = ['ils', 'ga', 'qvnd', 'gam', 'qvndm']
LABELS = {'ils': 'ILS+SA', 'ga': 'GA+RVND', 'qvnd': 'GA+QVND', 'gam': 'GA+RVND+Min', 'qvndm': 'GA+QVND+Min'}
CORES = {'ils': '#e41a1c', 'ga': '#377eb8', 'qvnd': '#4daf4a', 'gam': '#984ea3', 'qvndm': '#ff7f00'}
GENEROS = ['masculina', 'feminina']
TEMPORADAS = ['21-22', '22-23', '23-24', '24-25']

graficos_dir = os.path.join(DATADIR, 'graficos')
conv_media_dir = os.path.join(DATADIR, 'convergencia-media')
os.makedirs(graficos_dir, exist_ok=True)
os.makedirs(conv_media_dir, exist_ok=True)

# ========== CONVERGÊNCIA MÉDIA + GRÁFICOS ==========

for genero in GENEROS:
    for temp in TEMPORADAS:
        fig, ax = plt.subplots(figsize=(8, 5))

        for metodo in METODOS:
            dfs = []
            for k in range(1, 11):
                nome_arq = f'convergencia-{metodo}-{genero}-{temp}-exec{k}.csv'
                # Procurar na raiz ou na subpasta
                arq = os.path.join(DATADIR, nome_arq)
                if not os.path.exists(arq):
                    arq = os.path.join(DATADIR, f'convergencia-{metodo}-{genero}', nome_arq)
                if os.path.exists(arq):
                    df = pd.read_csv(arq, sep=';')
                    dfs.append(df)

            if not dfs:
                continue

            # Alinhar por TEMPO e calcular média
            merged = dfs[0][['TEMPO']].copy()
            for i, df in enumerate(dfs):
                merged = merged.merge(df.rename(columns={'FO': f'FO_{i}'}), on='TEMPO', how='outer')
            merged = merged.sort_values('TEMPO').reset_index(drop=True)

            fo_cols = [c for c in merged.columns if c.startswith('FO_')]
            merged['FO_media'] = merged[fo_cols].mean(axis=1)
            merged['FO_min'] = merged[fo_cols].min(axis=1)
            merged['FO_max'] = merged[fo_cols].max(axis=1)

            # Salvar convergência média/min/max em CSV
            media_csv = os.path.join(conv_media_dir, f'conv-media-{metodo}-{genero}-{temp}.csv')
            merged[['TEMPO', 'FO_media', 'FO_min', 'FO_max']].to_csv(media_csv, sep=';', index=False)
            print(f'Convergência média: {media_csv}')

            ax.plot(merged['TEMPO'], merged['FO_media'], label=LABELS[metodo], color=CORES[metodo], linewidth=1.5)
            ax.fill_between(merged['TEMPO'], merged['FO_min'], merged['FO_max'], color=CORES[metodo], alpha=0.12)

        ax.set_xlabel('Tempo (s)')
        ax.set_ylabel('FO média')
        ax.set_title(f'{genero.capitalize()} {temp}')
        ax.legend(fontsize=9)
        ax.grid(True, alpha=0.3)
        fig.tight_layout()

        nome = os.path.join(graficos_dir, f'convergencia-{genero}-{temp}.png')
        fig.savefig(nome, dpi=150)
        plt.close(fig)
        print(f'Gráfico: {nome}')

        # Gráfico individual por método (com faixa min-max)
        for metodo in METODOS:
            dfs = []
            for k in range(1, 11):
                nome_arq = f'convergencia-{metodo}-{genero}-{temp}-exec{k}.csv'
                arq = os.path.join(DATADIR, nome_arq)
                if not os.path.exists(arq):
                    arq = os.path.join(DATADIR, f'convergencia-{metodo}-{genero}', nome_arq)
                if os.path.exists(arq):
                    dfs.append(pd.read_csv(arq, sep=';'))
            if not dfs:
                continue

            merged = dfs[0][['TEMPO']].copy()
            for i, df in enumerate(dfs):
                merged = merged.merge(df.rename(columns={'FO': f'FO_{i}'}), on='TEMPO', how='outer')
            merged = merged.sort_values('TEMPO').reset_index(drop=True)
            fo_cols = [c for c in merged.columns if c.startswith('FO_')]
            merged['FO_media'] = merged[fo_cols].mean(axis=1)
            merged['FO_min'] = merged[fo_cols].min(axis=1)
            merged['FO_max'] = merged[fo_cols].max(axis=1)

            fig2, ax2 = plt.subplots(figsize=(8, 5))
            ax2.plot(merged['TEMPO'], merged['FO_media'], color=CORES[metodo], linewidth=1.5, label='Média')
            ax2.fill_between(merged['TEMPO'], merged['FO_min'], merged['FO_max'], color=CORES[metodo], alpha=0.2, label='Min-Max')
            ax2.set_xlabel('Tempo (s)')
            ax2.set_ylabel('FO')
            ax2.set_title(f'{LABELS[metodo]} — {genero.capitalize()} {temp}')
            ax2.legend(fontsize=9)
            ax2.grid(True, alpha=0.3)
            fig2.tight_layout()

            nome2 = os.path.join(graficos_dir, f'convergencia-{metodo}-{genero}-{temp}.png')
            fig2.savefig(nome2, dpi=150)
            plt.close(fig2)
            print(f'Gráfico individual: {nome2}')

# ========== TABELA COMPARATIVA ==========

print('\n' + '='*90)
print(f'{"Instância":<18} {"Método":<14} {"fbest":>8} {"favg":>8} {"Δ%":>6} {"ω(s)":>6}')
print('='*90)

rows = []
for genero in GENEROS:
    for temp in TEMPORADAS:
        for metodo in METODOS:
            arq = os.path.join(DATADIR, f'resultados-{metodo}.csv')
            if not os.path.exists(arq):
                continue
            df = pd.read_csv(arq, sep=';')
            filtro = df[(df['EDICAO'] == genero) & (df['TEMPORADA'] == temp)]
            if filtro.empty:
                continue
            fbest = filtro['FO'].min()
            favg = filtro['FO'].mean()
            delta = 100 * (favg - fbest) / fbest
            omega = filtro['TEMPO MELHOR FO'].mean()
            print(f'{genero} {temp:<10} {LABELS[metodo]:<14} {fbest:>8d} {favg:>8.0f} {delta:>5.1f}% {omega:>5.0f}s')
            rows.append({'EDICAO': genero, 'TEMPORADA': temp, 'METODO': LABELS[metodo],
                         'fbest': fbest, 'favg': round(favg), 'delta_pct': round(delta, 1), 'omega_s': round(omega)})
        print('-'*90)

# Salvar tabela comparativa em CSV
tabela_csv = os.path.join(DATADIR, 'tabela-comparativa.csv')
pd.DataFrame(rows).to_csv(tabela_csv, sep=';', index=False)
print(f'\nTabela salva: {tabela_csv}')
print('Concluído!')
