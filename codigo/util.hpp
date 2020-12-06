#ifndef _UTIL_H
#define _UTIL_H

#include <iostream>
#include <vector>

using namespace std;

#define TAM_MAX_ARQ 100000000
#define UNI_ALOCACAO 4000
#define NUM_BLOCOS 24961
#define TAM_BLOCO 5
#define TAM_BITMAP 28000
#define TAM_FAT 128000
#define TAM_ENDERECO 8

#define BLOCO_NULO 54321
#define BLOCO_INVALIDO -1
#define CHAR_NULO ' '
#define SEPARADOR '|'

#define T_CRIADO 1
#define T_MODIFICADO 2
#define T_ACESSO 4
#define T_TODOS T_MODIFICADO + T_CRIADO + T_ACESSO

#define DEBUG false

class Escrevivel {
  public:
    virtual void carrega(int) = 0;
    virtual void salva() = 0;
};

// string representando o disco
extern string discoAtual;

// Nome do arquivo que contém o disco salvo
extern string nomeArquivo;

// Pega as mudanças realizadas até agora e atualiza o disco
void atualizaDisco();

int blocoEmEndereco(int);
int enderecoEmBloco(int);

int blocoEmBaseLimite(int, int &, int &);

string intParaString(int, int);

string intParaMes(int);
string bytesFormatados(int);
void divideEmArquivoECaminho(string &, string &);

void limpaBloco(int numBloco);
void limpaBloco(int ende, int numBloco);

class ArquivoGenerico;
ArquivoGenerico *caminhoParaArquivo(string caminho);

// A quantidade atual de arquivos regulares criados
extern int qntArquivos;

// A quantidade atual de diretórios criados
extern int qntDiretorios;

// A quantidade de espaço desperdiçado
extern int espacoDesperdicadoTotal;

#endif
