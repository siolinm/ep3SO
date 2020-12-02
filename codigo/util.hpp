#ifndef _UTIL_H
#define _UTIL_H

#include <bits/types/time_t.h>

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define TAM_MAX_ARQ 100000000
#define UNI_ALOCACAO 4000
#define NUM_BLOCOS 24961
#define TAM_BLOCO 5
#define TAM_BITMAP 28000
#define TAM_FAT 128000
#define TAM_ENDERECO 8

#define BLOCO_NULO -1
#define CHAR_NULO ' '

#define LIVRE true
#define OCUPADO false

#define T_CRIADO 1
#define T_MODIFICADO 2
#define T_ACESSO 4
#define T_TODOS T_MODIFICADO + T_CRIADO + T_ACESSO

class Escrevivel {
  public:
    virtual void carrega(int) = 0;
    virtual void salva() = 0;
};

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
    /* TODO: Fazer algo pra limitar o nome <23-11-20, Lucas> */

  public:
    ArquivoInfo *informacoes;

    void carrega(int);
    void salva();
};

class Arquivo : public ArquivoGenerico {
  public:
    // A quantidade atual de arquivos regulares criados
    static int qntArquivos;

    int tamanho;     // Tamanho em bytes
    string conteudo; // Conteúdo do arquivo

    void carrega(int);
    void salva();
};

class Diretorio : public ArquivoGenerico {
  public:
    // A quantidade atual de diretórios criados
    static int qntDiretorios;

    vector<ArquivoInfo> subArquivoInfo;
    vector<Diretorio *> subDiretorio;
    vector<Arquivo *> subArquivo;

    string heap; // Heap com os nomes de cada arquivo

    virtual void carrega(int);
    virtual void salva();
    ArquivoGenerico *busca(string);
    bool buscaAbaixo(string, string);
};

class Root : public Diretorio {
  public:
    ArquivoInfo informacoes;
    void inicializa();
    void carrega(int);
    void salva();
};

class FAT_t : public Escrevivel {
  public:
    void inicializa();

    int ponteiro[NUM_BLOCOS];

    void carrega(int);
    void salva();
    void liberaBlocos(int);

    int alocaBloco(int);
};

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

// string representando o disco
extern string discoAtual;

// Nome do arquivo que contém o disco salvo
extern string nomeArquivo;

extern FAT_t FAT;
extern Bitmap_t Bitmap;
extern Root root;

// Pega as mudanças realizadas até agora e atualiza o disco
void atualizaDisco();

int blocoEmEndereco(int);
int enderecoEmBloco(int);

int blocoEmBaseLimite(int, int &, int &);

string intParaString(int, int);

string intParaMes(int inteiro);
string bytesFormatados(int tamanhoBytes);

ArquivoGenerico* caminhoParaArquivo(string caminho);

#endif
