#!/bin/bash

rm arq1mb
rm arq10mb
rm arq30mb
python geraArq.py arq1mb 1000000
python geraArq.py arq10mb 10000000
python geraArq.py arq30mb 30000000

for i in ${1..8}; do
    for j in ${1..3}; do
       for k in ${1..30}; do
        rm sistema.disco # ver se dá pra copiar por cima
        cp $j.disco sistema.disco

        { time ./ep3 < $i.teste }

            $i.teste $j.disco
        done
    done
done

