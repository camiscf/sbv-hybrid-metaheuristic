#!/bin/bash
# Análise extra: 3 instâncias × 5 métodos × 10 exec × 600s
# Para verificar se os métodos continuam melhorando ou estabilizam

OUTDIR="resultados-600s"
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

# 3 instâncias representativas (1 masc grande, 1 fem pequena, 1 fem média)
INSTANCIAS=("masculina 22-23" "feminina 21-22" "feminina 23-24")
METODOS=("ils" "ga" "qvnd" "gam" "qvndm")

run_method() {
    local metodo=$1
    local logfile="${OUTDIR}/log-${metodo}.txt"

    # Limpar CSVs antigos do mesmo método que possam existir em src/
    rm -f "resultados-${metodo}.csv"

    echo "=== Iniciando ${metodo} em $(date) ===" > "$logfile"

    for inst in "${INSTANCIAS[@]}"; do
        local ed=$(echo "$inst" | cut -d' ' -f1)
        local temp=$(echo "$inst" | cut -d' ' -f2)

        # Limpar convergências antigas deste método/instância
        rm -f convergencia-${metodo}-${ed}-${temp}-exec*.csv

        echo ">>> ${metodo}: ${ed} ${temp} - $(date)" >> "$logfile"
        ./tcc2 "$ed" "$temp" 10 "$metodo" --tempo 600 >> "$logfile" 2>&1

        # Mover convergência e solução para OUTDIR
        mv convergencia-${metodo}-${ed}-${temp}-exec*.csv "${OUTDIR}/" 2>/dev/null
        mv solucao-${metodo}-${ed}-${temp}.txt "${OUTDIR}/" 2>/dev/null
    done

    # Mover CSV de resultados para OUTDIR
    mv "resultados-${metodo}.csv" "${OUTDIR}/" 2>/dev/null

    # Limpar cabeçalhos duplicados
    local csv="${OUTDIR}/resultados-${metodo}.csv"
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

echo "=== 5 métodos em paralelo, 3 instâncias, 600s — $(date) ==="

for m in "${METODOS[@]}"; do
    run_method "$m" &
    eval "PID_${m}=$!"
done

echo "Todos rodando. Acompanhe com: tail -f ${OUTDIR}/log-*.txt"
echo "Progresso: wc -l ${OUTDIR}/resultados-*.csv"

for m in "${METODOS[@]}"; do
    eval "wait \$PID_${m}"
    echo "${m} finalizado: $(date)"
done

echo "=== Gerando convergência média e gráficos... ==="
python3 plot_resultados.py "$OUTDIR"

echo "Tudo concluído: $(date)"
