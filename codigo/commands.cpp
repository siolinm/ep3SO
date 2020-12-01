#include "commands.hpp"

#include <fstream>
#include <sstream>


void mount() {
    cin >> nomeArquivo;

    ifstream arquivo(nomeArquivo);
    stringstream buffer;

    if (!arquivo.is_open()) {
        ofstream arqTmp(nomeArquivo);
        string aux(TAM_MAX_ARQ, CHAR_NULO);
        discoAtual = aux;

        Bitmap.inicializa();
        FAT.inicializa();
        root.inicializa();
        Bitmap.salva();
        FAT.salva();
        root.salva();

        arqTmp << discoAtual;
        arqTmp.close();

        arquivo.open(nomeArquivo);
    }

    buffer << arquivo.rdbuf();
    discoAtual = buffer.str();
    arquivo.close();

    // Criamos a árvore de arquivos na memória principal
    root.carrega(0);
}

void cp() {
    string origem, destino;
    cin >> origem >> destino;

    cout << origem << " " << destino << "\n";
}

void mkdir() {
    string diretorio;
    cin >> diretorio;

    // cout << diretorio << "\n";
}

void rmdir() {
    string diretorio;
    cin >> diretorio;

    cout << diretorio << "\n";
}

void cat() {
    string arquivo;
    cin >> arquivo;

    cout << arquivo << "\n";
}

void touch() {
    string arquivo;
    cin >> arquivo;

    cout << arquivo << "\n";
}

void rm() {
    string arquivo;
    cin >> arquivo;

    cout << arquivo << "\n";
}

void ls() {
    string diretorio;
    cin >> diretorio;

    cout << diretorio << "\n";
}

void find() {
    string diretorio, arquivo;
    cin >> diretorio >> arquivo;

    cout << diretorio << " " << arquivo << "\n";
}

void df() { }

void umount() {
    ofstream arquivo(nomeArquivo);
    arquivo << discoAtual;
    // Free em tudo os bagulho
    arquivo.close();
}
