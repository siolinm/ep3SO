#!/bin/bash

DIR=testes
DIR_RELATORIO=$DIR/relatorios
DIR_TEMPO=$DIR/tempos

echo "Gerando arquivo arq1mb..."
python3 $DIR/geraArq.py $DIR/arq1mb 1000000
echo "Gerando arquivo arq10mb..."
python3 $DIR/geraArq.py $DIR/arq10mb 10000000
echo "Gerando arquivo arq30mb..."
python3 $DIR/geraArq.py $DIR/arq30mb 30000000
echo "Gerando arquivo completa30mb..."
python3 $DIR/geraArq.py $DIR/completa10mb 7360000
echo "Gerando arquivo completa50mb..."
python3 $DIR/geraArq.py $DIR/completa50mb 47360000

for i in {1..10}; do
    echo "Gerando arquivo arq${i}kb..."
    python3 $DIR/geraArq.py "${DIR}/arq${i}kb" "${i}000"
done

make
for i in {1..3}; do
    echo "Gerando disco ${i}..."
    rm $i.disco
    ./ep3 <$DIR/geraDisco$i
done

if [ ! -d "$DIR_TEMPO" ]; then
    mkdir $DIR_TEMPO
fi
if [ ! -d "$DIR_RELATORIO" ]; then
    mkdir $DIR_RELATORIO
fi

# for i in {1..3}; do
    # j=1
    # for k in {1..30}; do
        # echo "Realizando teste ${k} da operacao ${i}, com sistema de arquivos ${j}..."
        # cp $j.disco sistema.disco
        # { time ./ep3 < $DIR/$i.teste >$DIR_RELATORIO/relatorio-$i-$j-$k.txt
        # } 2> $DIR_TEMPO/tempo-$i-$j-$k.txt
    # done
# done

for j in {1..3}; do
    for k in {1..30}; do
        cp $j.disco sistema.disco
        for i in {1..8}; do
            echo "Realizando teste $k, com sistema de arquivos $j e operacao $i..."
            { time ./ep3 < $DIR/$i.teste > /dev/null
            } 2> $DIR_TEMPO/tempo-$i-$j-$k.txt
        done
    done
done
rm *.disco
