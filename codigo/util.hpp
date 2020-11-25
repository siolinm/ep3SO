#ifndef _UTIL_H
#define _UTIL_H

#include <bits/types/time_t.h>

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define MOUNT "mount"
#define COPY "cp"
#define MKDIR "mkdir"
#define RMDIR "rmdir"
#define CAT "cat"
#define TOUCH "touch"
#define RM "rm"
#define LS "ls"
#define FIND "find"
#define DF "df"
#define UMOUNT "umount"
#define EXIT "sai"

#define UNI_ALOCACAO 4000
#define NUM_BLOCOS 100000000 / (UNI_ALOCACAO + 6)
#define TAM_BLOCO 5
#define TAM_BITMAP NUM_BLOCOS
#define TAM_FAT TAM_BLOCO *NUM_BLOCOS
#define TAM_ENDERECO 8

#define BLOCO_NULO -1
#define CHAR_NULO 0

class Escrevivel {
  public:
    virtual void carrega(int) = 0;
    virtual void salva() = 0;
};

class ArquivoGenerico : public Escrevivel {
  protected:
    const int TAM_TEMPO = 10;
    const int TAM_TAMANHO = 8;
    const int TAM_NOME = 255;
    /* TODO: Fazer algo pra limitar o nome <23-11-20, Lucas> */


  public:
    string nome;            // Nome do arquivo
    time_t tempoCriado;     // Instante criado
    time_t tempoModificado; // Última modificação no arquivo
    time_t tempoAcesso;     // Tempo de último acesso ao arquivo
    // Para as três variáveis acima, é preciso converter usando uma função para
    // obtermos a data exata (ctime).
    int numPrimeiroBloco; // Ponteiro para o bloco cabeça do arquivo

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

class ArquivoInfo {
  public:
    int ptNome;             // Ponteiro para o nome no heap
    string nome;            // O nome do arquivo
    int tamanho;            // Tamanho em bytes (se for dir, será 0)
    char ehDiretorio;       // 'D' se é um diretório e 'A' se é um arquivo
    time_t tempoCriado;     // Instante criado
    time_t tempoModificado; // Última modificação no arquivo
    time_t tempoAcesso;     // Tempo de último acesso ao arquivo
    int numPrimeiroBloco;   // Ponteiro para o bloco cabeça do arquivo
};

class Diretorio : public ArquivoGenerico {
  public:
    // A quantidade atual de diretórios criados
    static int qntDiretorios;

    vector<ArquivoInfo> subArquivo;
    // O diretório aponta para o endereço do bloco dos subdiretório
    string heap; // Heap com os nomes de cada arquivo

    void carrega(int);
    void salva();
};

class FAT_t : public Escrevivel {
  public:
    int ponteiro[TAM_FAT];

    void carrega(int);
    void salva();

    int alocaBloco(int);
};

class Bitmap_t : public Escrevivel {
  public:
    bool cheio[TAM_BITMAP]; // True -> Vazio | False -> Cheio

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

// Pega as mudanças realizadas até agora e atualiza o disco
void atualizaDisco();

int blocoEmEndereco(int);
int enderecoEmBloco(int);

int blocoEmBaseLimite(int, int &, int &);

string intParaString(int, int);

#endif
