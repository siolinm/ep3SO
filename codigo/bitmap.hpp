#ifndef _BITMAP_H
#define _BITMAP_H

#include "util.hpp"

#define LIVRE true
#define OCUPADO false

class Bitmap_t : public Escrevivel {
  public:
    void inicializa();

    bool livre[NUM_BLOCOS]; // True -> Vazio | False -> Cheio

    void carrega(int);
    void salva();

    // Ponteiro para o próximo bloco de memória livre
    int proxLivre;

    // Quantidade de blocos livres
    int blocosLivres;

    // Função que devolve o próximo bloco livre ou uma exceção caso não tenha
    // nenhum bloco livre
    int pegaProxLivre();
};

extern Bitmap_t Bitmap;

#endif /* ifndef _BITMAP_H */
