#include "commands.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
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
    string diretorio, nomeDir;
    cin >> diretorio;

    nomeDir = "";

    if (diretorio.back() == '/') diretorio.pop_back();

    while (diretorio.back() != '/') {
        nomeDir.push_back(diretorio.back());
        diretorio.pop_back();
    }

    if (diretorio.size() != 1)
        diretorio.pop_back();

    reverse(nomeDir.begin(), nomeDir.end());

    Diretorio *dirPai = (Diretorio *) caminhoParaArquivo(diretorio);
    if (dirPai != nullptr) {
        Diretorio *novo = new Diretorio(nomeDir, 0);
        novo->informacoes->pai = dirPai;
        dirPai->adiciona(novo);
    } else {
        cout << "Caminho inexistente" << endl;
    }
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

    Diretorio *dir = (Diretorio *) caminhoParaArquivo(diretorio);

    cout << "   Size Tempo Criado  Tempo Modifi  Tempo Acesso   Name";
    cout << endl;

    for (ArquivoInfo *arqInfo : dir->subArquivoInfo) arqInfo->imprimeInfos();
}

void find() {
    Diretorio *dir;
    string diretorio, arquivo;

    cin >> diretorio >> arquivo;

    dir = (Diretorio *) caminhoParaArquivo(diretorio);

    if (!dir->buscaAbaixo(diretorio, arquivo))
        cout << "Arquivo não encontrado" << endl;
}

void df() { }

void umount() {
    // Salvamos toda a árvore de arquivos para a string discoAtual, o fat e
    // bitmap
    Bitmap.salva();
    FAT.salva();
    root.salva();

    ofstream arquivo(nomeArquivo);
    arquivo << discoAtual;

    // Free em tudo os bagulho
    arquivo.close();
}
