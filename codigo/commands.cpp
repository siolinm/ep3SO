#include "commands.hpp"

#include<iomanip>
#include <algorithm>
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
    root.informacoes.imprimeInfos();
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

    if(diretorio.back() == '/')
        diretorio.pop_back();

    while(diretorio.back() != '/'){
        nomeDir.push_back(diretorio.back());
        diretorio.pop_back();
    }

    diretorio.pop_back();

    reverse(nomeDir.begin(), nomeDir.end());

    Diretorio * dirPai = (Diretorio *)caminhoParaArquivo(diretorio);
    if(dirPai != nullptr){
        Diretorio * novo = new Diretorio();
        dirPai->adiciona(novo);
    }
    else{
        cout << "Caminho inexistente" << endl;
    }
    // "/marcos/lucas/davi/novodir"

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

    Diretorio * dir = (Diretorio *)caminhoParaArquivo(diretorio);

    cout << " Size Tempo Criado  Tempo Modifi  Tempo Acesso   Name"; 
    cout << endl;

    for (ArquivoInfo & arq : dir->subArquivoInfo)
        arq.imprimeInfos();    
}

void find() {
    Diretorio * dir;
    string diretorio, arquivo;

    cin >> diretorio >> arquivo;

    dir = (Diretorio *)caminhoParaArquivo(diretorio);
    
    if(!dir->buscaAbaixo(diretorio, arquivo))
        cout << "Arquivo não encontrado" << endl;
    
}

void df() { }

void umount() {
    ofstream arquivo(nomeArquivo);
    arquivo << discoAtual;
    // Free em tudo os bagulho
    arquivo.close();
}
