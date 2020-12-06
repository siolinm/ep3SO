#!/bin/bash

DIR=testes/tempos

rm data.csv

# sistema_de_arquivos.operacao
for j in {1..3}; do
    for i in {1..8}; do
        echo -n "$j.$i," >> tmp
    done
done
echo "" >> tmp

for k in {1..30}; do
    # j=1
    # for i in {1..3}; do
        # { cat $DIR/tempo-$i-$j-$k.txt | grep real | sed 's/0m//' | sed 's/s//' | sed 's/,/./' | sed 's/real//' | sed 's/\s*//' | sed -z 's/\n//'
        # } >> tmp
        # echo -n "," >> tmp
        # echo $i-$j-$k
    # done

    for j in {1..3}; do
        for i in {1..8}; do
            { cat $DIR/tempo-$i-$j-$k.txt | grep real | sed 's/0m//' | sed 's/s//' | sed 's/,/./' | sed 's/real//' | sed 's/\s*//' | sed -z 's/[\n]//'
            } >> tmp
            echo -n "," >> tmp
            echo $i-$j-$k
        done
    done
    echo "" >> tmp
done

cat tmp | sed 's/,$//' > data.csv
rm tmp
