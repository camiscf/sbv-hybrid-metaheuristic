#!/bin/bash
# Roda os 5 métodos em 2 lotes: ILS/GA/QVND depois GAM/QVNDM
# 8 instâncias × 10 execuções × 900s cada
# Parâmetros calibrados via irace (2026-04-04)

OUTDIR="resultados-900s"
mkdir -p "$OUTDIR"

EDICOES=("masculina" "feminina")
TEMPORADAS=("21-22" "22-23" "23-24" "24-25")

run_method() {
    local metodo=$1
    local csv="${OUTDIR}/resultados-${metodo}.csv"
    local logfile="${OUTDIR}/log-${metodo}.txt"

    rm -f "$csv"

    echo "=== Iniciando ${metodo} em $(date) ===" > "$logfile"

    for ed in "${EDICOES[@]}"; do
        for temp in "${TEMPORADAS[@]}"; do
            echo ">>> ${metodo}: ${ed} ${temp} - $(date)" >> "$logfile"
            ./tcc2 "$ed" "$temp" 10 "$metodo" >> "$logfile" 2>&1

            # Mover arquivos de convergência para a pasta de saída
            for f in convergencia-${metodo}-${ed}-${temp}-exec*.csv; do
                [ -f "$f" ] && mv "$f" "${OUTDIR}/"
            done
        done
    done

    # Limpar cabeçalhos duplicados no CSV (manter só o primeiro)
    if [ -f "$csv" ]; then
        head -1 "$csv" > "${csv}.tmp"
        grep -v "^EDICAO" "$csv" >> "${csv}.tmp"
        mv "${csv}.tmp" "$csv"
    fi

    echo "=== Finalizado ${metodo} em $(date) ===" >> "$logfile"
}

echo "Compilando..."
g++ tcc2.cpp -O2 -std=c++17 -o tcc2
if [ $? -ne 0 ]; then
    echo "Erro de compilação!"
    exit 1
fi

echo "=== LOTE 1: ILS, GA, QVND — $(date) ==="

run_method "ils" &
PID_ILS=$!

run_method "ga" &
PID_GA=$!

run_method "qvnd" &
PID_QVND=$!

echo "PIDs: ILS=$PID_ILS, GA=$PID_GA, QVND=$PID_QVND"
echo "Acompanhe com: tail -f ${OUTDIR}/log-ils.txt"
echo "Progresso CSV: wc -l ${OUTDIR}/resultados-*.csv"

wait $PID_ILS
echo "ILS finalizado: $(date)"

wait $PID_GA
echo "GA finalizado: $(date)"

wait $PID_QVND
echo "QVND finalizado: $(date)"

echo "=== LOTE 2: GAM, QVNDM — $(date) ==="

run_method "gam" &
PID_GAM=$!

run_method "qvndm" &
PID_QVNDM=$!

echo "PIDs: GAM=$PID_GAM, QVNDM=$PID_QVNDM"
echo "Acompanhe com: tail -f ${OUTDIR}/log-gam.txt"

wait $PID_GAM
echo "GAM finalizado: $(date)"

wait $PID_QVNDM
echo "QVNDM finalizado: $(date)"

echo "=== Gerando convergência média e gráficos... ==="
python3 plot_resultados.py "$OUTDIR"

echo "Tudo concluído: $(date)"
