#ifndef _FAT_H
#define _FAT_H value

#include "util.hpp"

class FAT_t : public Escrevivel {
  public:
    void inicializa();

    int ponteiro[NUM_BLOCOS];

    void carrega(int);
    void salva();
    void liberaBlocos(int);

    int alocaBloco();
    int alocaBloco(int);
};

extern FAT_t FAT;

#endif /* ifndef _FAT_H */
