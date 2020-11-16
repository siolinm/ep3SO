#ifndef _UTIL_H
#define _UTIL_H

#include <bits/types/time_t.h>

#include <ctime>
#include <iostream>
#include <string>

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

#define MAX_SUB_ARQUIVOS 100
#define TAM_FAT 100000
#define TAM_BITMAP 10000

class arquivo {
  public:
    string nome; // Nome do arquivo
    bool ehDir;  // True se for um diretório
    int tamanho; // Tamanho em bytes (cabe em int pois 100MB = 1e8 bytes)
    time_t tempo_criado;     // Instante criado
    time_t tempo_modificado; // Última modificação no arquivo
    time_t tempo_acesso;     // Tempo de último acesso ao arquivo
    // Para as três variáveis acima, é preciso converter usando uma função para
    // obtermos a data exata (ctime).
    string conteudo;                         // Conteúdo do arquivo
    arquivo *sub_arquivos[MAX_SUB_ARQUIVOS]; // Lista de arquivos do diretório
    /* TODO: Pensar dps se deve ser um ponteiro para um arquivo em si ou se
     * deve ser para a posição da memória do arquivo <16-11-20, Lucas> */
};

/* TODO: Pensar se precisa mesmo das classes <16-11-20, Lucas> */
class FAT {
  public:
    /* TODO: Ideia de transformar em um uint isso aqui <16-11-20, Lucas> */
    int ponteiros[TAM_FAT];
};

class bitmap {
  public:
    bool cheio[TAM_BITMAP]; // True -> Vazio | False -> Cheio
};


#endif
