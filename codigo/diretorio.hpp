#ifndef _DIRETORIO_H
#define _DIRETORIO_H

#include <string>
#include <vector>

#include "arquivo.hpp"
#include "arquivoGenerico.hpp"

class Diretorio : public ArquivoGenerico {
  public:
    vector<ArquivoInfo *> subArquivoInfo;
    vector<Diretorio *> subDiretorio;
    vector<Arquivo *> subArquivo;

    Diretorio();
    Diretorio(string);
    Diretorio(string, int);
    ~Diretorio();

    virtual void carrega(int);
    virtual void salva();
    void salvaTudo();

    void adiciona(Arquivo *);
    void adiciona(Diretorio *);
    void remove(Arquivo *);
    void remove(Diretorio *);

    ArquivoGenerico *busca(string);
    bool buscaAbaixo(string, string);

    void libera();
};

class Root : public Diretorio {
  public:
    void inicializa();
    void carrega(int);
    void salva();

    Root();
    ~Root();
};

extern Root root;

#endif /* ifndef _DIRETORIO_H */

