#!/bin/bash

DIR=testes

echo "Gerando arquivo arq1mb..."    
python3 $DIR/geraArq.py $DIR/arq1mb 1000000
echo "Gerando arquivo arq10mb..."    
python3 $DIR/geraArq.py $DIR/arq10mb 10000000
echo "Gerando arquivo arq30mb..."    
python3 $DIR/geraArq.py $DIR/arq30mb 30000000

for i in {1..10}; do
    echo "Gerando arquivo arq${i}kb..."    
    python3 $DIR/geraArq.py "${DIR}/arq${i}kb" "${i}000"
done

for i in {1..3}; do
    echo "Gerando disco ${i}..."
    rm $i.disco
    ./ep3 <$DIR/geraDisco$i    
done

for i in {1..8}; do
    for j in {1..3}; do
        for k in {1..30}; do
            echo "Realizando teste ${k} da operacao ${i}, com sistema de arquivos ${j}..."
            cp $j.disco sistema.disco
            { time ./ep3 < $DIR/$i.teste >$DIR/relatorio-$i-$j-$k.txt 
            } 2> $DIR/teste-$i-$j-$k.txt
        done
    done
done