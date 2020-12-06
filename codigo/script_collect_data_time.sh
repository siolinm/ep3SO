#!/bin/bash

DIR=testes/tempos

rm data.csv

# sistema_de_arquivos.operacao
for j in {1..3}; do
    for i in {1..8}; do
        echo -n "$i.$j," >> data.csv
    done
done

for k in {1..30}; do
    # j=1
    # for i in {1..3}; do
        # { cat $DIR/tempo-$i-$j-$k.txt | grep real | sed 's/0m//' | sed 's/s//' | sed 's/,/./' | sed 's/real//' | sed 's/\s*//' | sed -z 's/\n//'
        # } >> data.csv
        # echo -n "," >> data.csv
        # echo $i-$j-$k
    # done

    for j in {1..3}; do
        for i in {1..8}; do
            { cat $DIR/tempo-$i-$j-$k.txt | grep real | sed 's/0m//' | sed 's/s//' | sed 's/,/./' | sed 's/real//' | sed 's/\s*//' | sed -z 's/[\n]//'
            } >> data.csv
            echo -n "," >> data.csv
            echo $i-$j-$k
        done
    done
    echo "" >> data.csv
done

cat data.csv | sed 's/,$//' > data.csv

