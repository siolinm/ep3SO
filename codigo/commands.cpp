#include "commands.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "util.hpp"

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
    Bitmap.carrega(0);
    FAT.carrega(0);
    root.carrega(0);
}

void cp() {
    string origem, destino, nomeArq;
    cin >> origem >> destino;

    ifstream arquivo(origem);
    stringstream str;
    str << arquivo.rdbuf();

    divideEmArquivoECaminho(destino, nomeArq);

    Diretorio *dirPai = (Diretorio *) caminhoParaArquivo(destino);

    if (dirPai == nullptr) {
        cerr << "Caminho inexistente" << endl;
        return;
    }

    Arquivo *novoArquivo = new Arquivo(nomeArq, str.str().size());
    if (novoArquivo->informacoes->numPrimeiroBloco == BLOCO_INVALIDO) {
        cerr << "O arquivo " << nomeArq << " não pôde ser criado." << endl;
        delete novoArquivo;
        return;
    }

    novoArquivo->conteudo = str.str();
    try {
        dirPai->adiciona(novoArquivo);
    } catch (string err) {
        cerr << "cp: Erro ao adicionar o arquivo " << nomeArq
             << " ao diretório " << dirPai->informacoes->nome << endl;
        cerr << "erro: " << err << endl;
        FAT.liberaBlocos(novoArquivo->informacoes->numPrimeiroBloco);
        delete novoArquivo;
        dirPai->salva();
    }
}

void mkdir() {
    string diretorio, nomeDir;
    cin >> diretorio;

    divideEmArquivoECaminho(diretorio, nomeDir);

    Diretorio *dirPai = (Diretorio *) caminhoParaArquivo(diretorio);

    if (dirPai == nullptr) {
        cerr << "Caminho inexistente" << endl;
        return;
    }

    Diretorio *novoDiretorio = new Diretorio(nomeDir, 0);
    if (novoDiretorio->informacoes->numPrimeiroBloco == BLOCO_INVALIDO) {
        cerr << "O diretório " << nomeDir << " não pôde ser criado." << endl;
        delete novoDiretorio;
        return;
    }

    try {
        dirPai->adiciona(novoDiretorio);
    } catch (string err) {
        cerr << "cp: Erro ao adicionar o diretório " << nomeDir
             << " ao diretório " << dirPai->informacoes->nome << endl;
        cerr << "erro: " << err << endl;
        FAT.liberaBlocos(novoDiretorio->informacoes->numPrimeiroBloco);
        delete novoDiretorio;
        dirPai->salva();
    }
}

void rmdir() {
    string diretorio;
    cin >> diretorio;

    if (diretorio.size() == 1) {
        cout << "Não é possível remover este diretório" << endl;
        return;
    }

    Diretorio *dir = (Diretorio *) caminhoParaArquivo(diretorio);

    if (dir == nullptr) {
        cerr << "Caminho inexistente" << endl;
        return;
    }

    dir->informacoes->pai->remove(dir);
}

void cat() {
    string arquivo;
    cin >> arquivo;

    Arquivo *arq = (Arquivo *) caminhoParaArquivo(arquivo);

    if (arq == nullptr) return;

    cout << arq->conteudo << endl;
}

void touch() {
    string arquivo, nomeArq;
    cin >> arquivo;

    Arquivo *arq = (Arquivo *) caminhoParaArquivo(arquivo);

    if (arq != nullptr) {
        /* altera data de acesso e modificacao */
        arq->informacoes->atualizaTempo(T_ACESSO + T_MODIFICADO);
    } else {
        divideEmArquivoECaminho(arquivo, nomeArq);

        Diretorio *dirPai = (Diretorio *) caminhoParaArquivo(arquivo);

        if (dirPai == nullptr) {
            cerr << "Caminho inexistente" << endl;
            return;
        }

        Arquivo *novoArquivo = new Arquivo(nomeArq);
        if (novoArquivo->informacoes->numPrimeiroBloco == BLOCO_INVALIDO) {
            cerr << "O arquivo " << nomeArq << " não pôde ser criado."
                 << endl;
            delete novoArquivo;
            return;
        }
        novoArquivo->conteudo = "";

        try {
            dirPai->adiciona(novoArquivo);
        } catch (string err) {
            cerr << "cp: Erro ao adicionar o arquivo " << nomeArq
                 << " ao diretório " << dirPai->informacoes->nome << endl;
            cerr << "erro: " << err << endl;
            FAT.liberaBlocos(novoArquivo->informacoes->numPrimeiroBloco);
            delete novoArquivo;
            dirPai->salva();
        }
    }
}

void rm() {
    string arquivo;
    cin >> arquivo;

    Arquivo *arq = (Arquivo *) caminhoParaArquivo(arquivo);

    if (arq == nullptr) {
        cerr << "Caminho inexistente" << endl;
        return;
    }

    arq->informacoes->pai->remove(arq);
}

void ls() {
    string diretorio;
    cin >> diretorio;

    Diretorio *dir = (Diretorio *) caminhoParaArquivo(diretorio);

    cout << "   Size Tempo Criado  Tempo Modifi  Tempo Acesso  Name";
    cout << endl;

    for (ArquivoInfo *arqInfo : dir->subArquivoInfo) arqInfo->imprimeInfos();
}

void find() {
    Diretorio *dir;
    string diretorio, arquivo;

    cin >> diretorio >> arquivo;

    dir = (Diretorio *) caminhoParaArquivo(diretorio);

    if (dir == nullptr || !dir->buscaAbaixo(diretorio, arquivo))
        cerr << "Arquivo não encontrado" << endl;
}

void df() {
    cout << "Quantidade de diretórios: " << qntDiretorios << endl;
    cout << "Quantidade de arquivos: " << qntArquivos << endl;
    cout << "Espaço Livre: " << Bitmap.blocosLivres * UNI_ALOCACAO << endl;
    cout << "Espaço Desperdiçado: " << espacoDesperdicadoTotal << endl;
}

void umount() {
    // Salvamos toda a árvore de arquivos para a string discoAtual, o fat e
    // bitmap
    Bitmap.salva();
    FAT.salva();
    root.salvaTudo();

    ofstream arquivo(nomeArquivo);
    arquivo << discoAtual;

    arquivo.close();
    root.libera();
}
