#!/bin/bash
# Pré-treino do Q-Learning em 30 instâncias dummy
# Uso: ./pre_treino.sh [max_geracoes] [q_table_output]
# Default: 150 gerações, salva em resultados-correcoes/q_table_pretrain.qtable

MAX_GER=${1:-150}
QTABLE=${2:-resultados-correcoes/q_table_pretrain.qtable}

echo "=== Pré-treino Q-Learning ==="
echo "Gerações por instância: $MAX_GER"
echo "Q-table de saída: $QTABLE"

# Gerar lista de dummies
ls dummies/dummy-*.txt > lista_dummies.txt
echo "Instâncias: $(wc -l < lista_dummies.txt) dummies"

# Compilar se necessário
if [ ! -f tcc2 ] || [ tcc2.cpp -nt tcc2 ]; then
    echo "Compilando tcc2..."
    g++ tcc2.cpp -O2 -std=c++17 -o tcc2
fi

mkdir -p resultados-correcoes

time ./tcc2 dummy dummy 1 qvnd \
    --modo-treino \
    --lista-instancias lista_dummies.txt \
    --q-table-path "$QTABLE" \
    --max-geracoes "$MAX_GER" \
    --seed 42

echo ""
echo "Pré-treino concluído. Q-table: $QTABLE ($(wc -c < "$QTABLE") bytes)"
