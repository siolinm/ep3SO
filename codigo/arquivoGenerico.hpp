#ifndef _ARQUIVO_GENERICO_H
#define _ARQUIVO_GENERICO_H value

#include <bits/types/time_t.h>

#include <ctime>
#include <string>

#include "util.hpp"

class Diretorio;
class ArquivoInfo {
  public:
    int ptNome;             // Ponteiro para o nome no heap
    string nome;            // O nome do arquivo
    int tamanho;            // Tamanho em bytes (se for dir, será 0)
    char ehDiretorio;       // 'D' se é um diretório e 'A' se é um arquivo
    time_t tempoCriado;     // Instante criado
    time_t tempoModificado; // Última modificação no arquivo
    time_t tempoAcesso;     // Tempo de último acesso ao arquivo
    int numPrimeiroBloco;   // Número do bloco cabeça do arquivo
    Diretorio *pai;         // Ponteiro para o diretório pai
    int espacoDesperdicado;

    ArquivoInfo();
    ArquivoInfo(string);
    ArquivoInfo(string, int);

    // Função que atualiza os tempos (criado, modificado, acesso) baseado numa
    // bit mask onde 001 é criado, 010 é modificado e 100 é acesso.
    // Ou seja, se passarmos o valor 5, então atualizaremos o tempo criado e de
    // acesso para o tempo atual. Se passarmos 7, atualizaremos tudo, se
    // passarmos qualquer valor maior que 7 ou menor que 1, não faremos nada.
    void atualizaTempo(int);

    void imprimeInfos();
};

class ArquivoGenerico : public Escrevivel {
  protected:
    const int TAM_TEMPO = 10;
    const int TAM_TAMANHO = 8;
    const int TAM_NOME = 255;

  public:
    ArquivoInfo *informacoes;

    ArquivoGenerico();
    ArquivoGenerico(string);
    ArquivoGenerico(string, int);

    virtual void carrega(int);
    virtual void salva();

    virtual ~ArquivoGenerico();
};

#endif /* ifndef _ARQUIVO_GENERICO_H */
