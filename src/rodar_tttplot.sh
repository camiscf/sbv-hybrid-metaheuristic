#!/bin/bash
# Roda o experimento TTT completo:
# 5 métodos × 8 instâncias × 30 execuções
# Paralelo com xargs -P 4
#
# Uso: ./rodar_tttplot.sh [num_exec] [max_geracoes]
# Default: 30 execuções, 150 gerações

NUM_EXEC=${1:-30}
MAX_GER=${2:-150}
QTABLE="resultados-correcoes/q_table_pretrain.qtable"
ALVOS_CSV="resultados-correcoes/alvos.csv"
OUTDIR="resultados-correcoes/ttt"

echo "=== Experimento TTT ==="
echo "Execuções por método/instância: $NUM_EXEC"
echo "Gerações (GA): $MAX_GER"

# Compilar se necessário
if [ ! -f tcc2 ] || [ tcc2.cpp -nt tcc2 ]; then
    echo "Compilando tcc2..."
    g++ tcc2.cpp -O2 -std=c++17 -o tcc2
fi

mkdir -p "$OUTDIR"

# Gerar lista de tarefas
TASKFILE=$(mktemp)

while IFS=, read -r edicao temporada alvo0 alvo2 alvo5; do
    # Pular header
    [[ "$edicao" == "edicao" ]] && continue

    ALVOS_STR="${alvo0},${alvo2},${alvo5}"

    for metodo in ils ga qvnd gam qvndm; do
        for exec_num in $(seq 1 $NUM_EXEC); do
            SAIDA="${OUTDIR}/${metodo}_${edicao}_${temporada}_exec${exec_num}.txt"

            # Construir comando base
            CMD="./tcc2 ${edicao} ${temporada} 1 ${metodo}"
            CMD="$CMD --seed ${exec_num}"
            CMD="$CMD --alvos ${ALVOS_STR}"
            CMD="$CMD --saida-ttt ${SAIDA}"
            CMD="$CMD --irace"

            # GA: parada por gerações
            if [ "$metodo" != "ils" ]; then
                CMD="$CMD --max-geracoes ${MAX_GER}"
            fi

            # QVND/QVNDM: warm-start com Q-table pré-treinada
            if [ "$metodo" == "qvnd" ] || [ "$metodo" == "qvndm" ]; then
                if [ -f "$QTABLE" ]; then
                    CMD="$CMD --q-table-path ${QTABLE}"
                fi
            fi

            echo "$CMD"
        done
    done
done < "$ALVOS_CSV" > "$TASKFILE"

TOTAL=$(wc -l < "$TASKFILE")
echo "Total de execuções: $TOTAL"
echo "Executando com 4 processos em paralelo..."

time cat "$TASKFILE" | xargs -P 4 -I {} bash -c '{}'

rm -f "$TASKFILE"

echo ""
echo "=== Concluído ==="
echo "Arquivos TTT: $(ls $OUTDIR/*.txt 2>/dev/null | wc -l)"
echo "Diretório: $OUTDIR/"
