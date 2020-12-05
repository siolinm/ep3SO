#!/bin/bash

DIR=testes/tempos

rm data.csv

# sistema_de_arquivos.operacao
echo "1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 3.1, 3.2, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8" >> data.csv
for k in {1..30}; do
    j=1
    for i in {1..3}; do
        { cat $DIR/tempo-$i-$j-$k.txt | grep real | sed 's/0m//' | sed 's/s//' | sed 's/,/./' | sed 's/real//' | sed 's/\s*//' | sed -z 's/\n//'
        } >> data.csv
        echo -n "," >> data.csv
        echo $i-$j-$k
    done

    for j in {2..3}; do
        for i in {1..8}; do
            { cat $DIR/tempo-$i-$j-$k.txt | grep real | sed 's/0m//' | sed 's/s//' | sed 's/,/./' | sed 's/real//' | sed 's/\s*//' | sed -z 's/[\n]//'
            } >> data.csv
            echo -n "," >> data.csv
            echo $i-$j-$k
        done
    done
    echo "" >> data.csv
done
