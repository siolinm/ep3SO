#ifndef _ARQUIVO_H
#define _ARQUIVO_H

#include <string>

#include "arquivoGenerico.hpp"

class Arquivo : public ArquivoGenerico {
  public:
    Arquivo();
    Arquivo(string);
    Arquivo(string, int);

    string conteudo; // Conteúdo do arquivo

    void carrega(int);
    void salva();

    ~Arquivo();
};

#endif /* ifndef _ARQUIVO_H */
