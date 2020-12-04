#include "util.hpp"

#include <math.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <queue>
#include <string>
#include <vector>

string discoAtual;
FAT_t FAT;
Bitmap_t Bitmap;
Root root;
string nomeArquivo;
int qntArquivos = 0;
int qntDiretorios = 0;
int espacoDesperdicadoTotal = 0;

int blocoEmEndereco(int numBloco) {
    return TAM_BITMAP + TAM_FAT + numBloco * UNI_ALOCACAO;
}

int enderecoEmBloco(int endereco) {
    return (endereco - TAM_BITMAP - TAM_FAT) / UNI_ALOCACAO;
}

int blocoEmBaseLimite(int numBloco, int &base, int &limite) {
    int end = blocoEmEndereco(numBloco);
    base = end;
    limite = end + UNI_ALOCACAO;
    return end;
}

/*
 * void limpaBloco(int numBloco)
 *
 * limpa o conteudo do bloco (coloca CHAR_NULO)
 *
 */
void limpaBloco(int numBloco) {
    int ende, base, limite;
    ende = blocoEmBaseLimite(numBloco, base, limite);

    for (; ende < limite; ende++) discoAtual[ende] = CHAR_NULO;
}

void limpaBloco(int ende, int numBloco) {
    int base, limite;
    blocoEmBaseLimite(numBloco, base, limite);

    for (; ende < limite; ende++) discoAtual[ende] = CHAR_NULO;
}

string intParaString(int inteiro, int tamanho) {
    string ret;
    ret = to_string(inteiro);
    reverse(ret.begin(), ret.end());
    while ((int) ret.size() < tamanho) ret.push_back('0');
    reverse(ret.begin(), ret.end());
    return ret;
}

string intParaMes(int inteiro) {
    vector<string> meses { "jan", "fev", "mar", "abr", "mai", "jun",
                           "jul", "aug", "set", "out", "nov", "dez" };
    return meses[inteiro];
}

string bytesFormatados(int tamanhoBytes) {
    double tamanho = tamanhoBytes;
    stringstream ss;
    if (tamanho / 1e6 >= 1) {
        tamanho /= 1e6;
        if (tamanho - floor(tamanho) != 0) {
            tamanho = floor(tamanho * 10) / 10;
        }
        ss << tamanho;
        return ss.str().append("M");
    } else if (tamanho / 1e3 >= 1) {
        tamanho /= 1e3;

        if (tamanho > 100) tamanho = round(tamanho);

        if (tamanho - floor(tamanho) != 0) {
            tamanho = floor(tamanho * 10) / 10;
        }
        ss << tamanho;

        return ss.str().append("k");
    } else
        return to_string(tamanhoBytes);
}

void divideEmArquivoECaminho(string &caminho, string &nome) {
    nome = "";

    // Caminhos podem ou não ter barra no final, nós ignoramos-a se sim
    if (caminho.back() == '/') caminho.pop_back();

    while (caminho.back() != '/') {
        nome.push_back(caminho.back());
        caminho.pop_back();
    }

    // Sobrará a barra (e.g. /marcos/lucas/), então tiramos a menos que seja
    // apenas o diretório '/'.
    if (caminho.size() != 1) caminho.pop_back();

    reverse(nome.begin(), nome.end());
}

ArquivoInfo::ArquivoInfo() {
    atualizaTempo(T_TODOS);
    pai = nullptr;
    tamanho = 0;
    nome = "";
    ptNome = 0;
    ehDiretorio = '0';
    espacoDesperdicado = 0;
}

ArquivoInfo::ArquivoInfo(string nome) {
    atualizaTempo(T_TODOS);
    pai = nullptr;
    tamanho = 0;
    this->nome = nome;
    ptNome = 0;
    ehDiretorio = '0';
    espacoDesperdicado = 0;
}

ArquivoInfo::ArquivoInfo(string nome, int tamanho) {
    atualizaTempo(T_TODOS);
    pai = nullptr;
    this->tamanho = tamanho;
    this->nome = nome;
    ptNome = 0;
    ehDiretorio = '0';
    espacoDesperdicado = UNI_ALOCACAO - (tamanho % UNI_ALOCACAO);
    espacoDesperdicadoTotal += espacoDesperdicado;
}

void ArquivoInfo::atualizaTempo(int bitmask) {
    if (bitmask < 1 && bitmask > 7) return;

    time_t tempoAtual = time(NULL);
    if (bitmask & 1) tempoCriado = tempoAtual;
    bitmask = bitmask >> 1;
    if (bitmask & 1) tempoModificado = tempoAtual;
    bitmask = bitmask >> 1;
    if (bitmask & 1) tempoAcesso = tempoAtual;
    bitmask = bitmask >> 1;
}

void ArquivoInfo::imprimeInfos() {
    auto imprimeTempo = [](time_t *tempo) {
        struct tm *timeInfo;
        timeInfo = localtime(tempo);
        cout << setw(2) << setfill('0') << timeInfo->tm_mday << " "
             << intParaMes(timeInfo->tm_mon) << " " << setw(2) << setfill('0')
             << timeInfo->tm_hour << ":" << setw(2) << setfill('0')
             << timeInfo->tm_min << "  ";
    };

    cout << ehDiretorio << " ";
    cout << setw(5)
         << ((ehDiretorio == 'D') ? "-----" : bytesFormatados(tamanho)) << " ";
    // TEMPORARIO: depois deixar apenas o tempo de última modificação
    for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
        imprimeTempo(tempo);

    cout << nome << endl;
}

ArquivoGenerico::~ArquivoGenerico() {
    // FAT.liberaBlocos(informacoes->numPrimeiroBloco);
    delete (informacoes);
}

ArquivoGenerico::ArquivoGenerico() { informacoes = nullptr; }

ArquivoGenerico::ArquivoGenerico(string nome) {
    informacoes = new ArquivoInfo(nome);
}

ArquivoGenerico::ArquivoGenerico(string nome, int tamanho) {
    informacoes = new ArquivoInfo(nome, tamanho);
}

ArquivoGenerico *caminhoParaArquivo(string caminho) {
    Diretorio *atual = &root;
    ArquivoGenerico *retorn = nullptr;

    int i;
    string nomeAtual;

    if (caminho.size() == 1) {
        if (caminho[0] == '/')
            return atual;
        else {
            cerr << "Caminho inexistente." << endl;
            return nullptr;
        }
    }

    i = 1; // ignoramos '/' no começo da string
    if (caminho[(int) caminho.size() - 1] == '/') caminho.pop_back();

    while (i < (int) caminho.size()) {
        nomeAtual = "";
        while (i < (int) caminho.size() && caminho[i] != '/')
            nomeAtual.push_back(caminho[i++]);

        if (i < (int) caminho.size()) {
            atual = (Diretorio *) atual->busca(nomeAtual);
            if (atual == nullptr) return nullptr;
        }
        i++;
    }

    retorn = atual->busca(nomeAtual);

    if (retorn == nullptr) {
        // cerr << "ERRO: Arquivo não encontrado!" << endl;
        // cerr << "Retornando arquivo nulo!" << endl;
        return nullptr;
    }

    return retorn;
}

void ArquivoGenerico::carrega(int numBloco) { }

void ArquivoGenerico::salva() { }

Arquivo::~Arquivo() {
    qntArquivos--;
}

Arquivo::Arquivo() : ArquivoGenerico() {
    qntArquivos++;
    conteudo = "";
}

Arquivo::Arquivo(string nome) : ArquivoGenerico(nome) {
    qntArquivos++;
    conteudo = "";
    informacoes->ehDiretorio = 'A';
    informacoes->numPrimeiroBloco = FAT.alocaBloco();
}

Arquivo::Arquivo(string nome, int tamanho) : ArquivoGenerico(nome, tamanho) {
    qntArquivos++;
    conteudo = "";
    informacoes->ehDiretorio = 'A';
    informacoes->numPrimeiroBloco = FAT.alocaBloco();
    tamanho -= 4000;
    while (tamanho > 0) {
        if (FAT.alocaBloco(informacoes->numPrimeiroBloco) == BLOCO_INVALIDO) {
            FAT.liberaBlocos(informacoes->numPrimeiroBloco);
            informacoes->numPrimeiroBloco = BLOCO_INVALIDO;
            break;
        }

        tamanho -= 4000;
    }
}

void Arquivo::carrega(int numBloco) {
    // Base e limite para o bloco que estamos atualmente carregando
    int base, limite;
    int ende; // Endereço atual sendo lido

    ende = blocoEmBaseLimite(numBloco, base, limite);

    if (DEBUG) cout << "Carregando arquivo do endereço " << ende << endl;

    conteudo = "";
    while (numBloco != BLOCO_NULO) {
        for (; ende < limite && discoAtual[ende] != SEPARADOR; ende++)
            conteudo.push_back(discoAtual[ende]);

        numBloco = FAT.ponteiro[numBloco]; // próximo bloco
        ende = blocoEmBaseLimite(numBloco, base, limite);
    }
}

void Arquivo::salva() {
    int base, limite;
    int ende; // Endereço atual sendo lido
    int numBloco = informacoes->numPrimeiroBloco;

    ende = blocoEmBaseLimite(numBloco, base, limite);

    if (DEBUG)
        cout << "Salvando o arquivo " << informacoes->nome << " na posição "
             << ende << endl;

    int i = 0; // posicao atual do conteudo
    while (numBloco != BLOCO_NULO) {
        for (; i < (int) conteudo.size() && ende < limite; i++, ende++)
            discoAtual[ende] = conteudo[i];

        if (ende < limite) discoAtual[ende++] = SEPARADOR;
        // se o conteudo acabou antes do limite do bloco, preenchemos com 'nulo'
        while (ende < limite) discoAtual[ende++] = CHAR_NULO;

        numBloco = FAT.ponteiro[numBloco]; // próximo bloco
        ende = blocoEmBaseLimite(numBloco, base, limite);
    }
}

Diretorio::Diretorio() : ArquivoGenerico() { qntDiretorios++; }

Diretorio::~Diretorio(){
    qntDiretorios--;
}

Diretorio::Diretorio(string nome) : ArquivoGenerico(nome) {
    qntDiretorios++;
    informacoes->ehDiretorio = 'D';
    informacoes->numPrimeiroBloco = FAT.alocaBloco();
}

Diretorio::Diretorio(string nome, int tamanho)
    : ArquivoGenerico(nome, tamanho) {
    qntDiretorios++;
    informacoes->ehDiretorio = 'D';
    informacoes->numPrimeiroBloco = FAT.alocaBloco();
}

void Diretorio::carrega(int numBloco) {
    int base, limite;
    int ende; // Endereço atual sendo lido

    auto pulaBloco = [&ende, &numBloco, &base, &limite]() {
        if (ende == limite) {
            numBloco = FAT.ponteiro[numBloco];
            ende = blocoEmBaseLimite(numBloco, base, limite);
        }
    };

    auto pegaTempo = [&ende](int tam) {
        string aux;
        aux = "";
        for (int i = 0; i < tam; i++, ende++) aux.push_back(discoAtual[ende]);
        return (time_t) stoi(aux);
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    if (DEBUG) cout << "Carregando diretório do endereço " << ende << endl;

    // Pegar as entradas de arquivo
    while (discoAtual[ende] != SEPARADOR) {
        ArquivoInfo *subArqInfo = new ArquivoInfo();
        subArquivoInfo.push_back(subArqInfo);

        // Pegar o ponteiro para o nome na heap
        string aux = "";
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo->ptNome = stoi(aux);

        // Pegar a flag de arquivo ou diretório
        pulaBloco();
        subArqInfo->ehDiretorio = discoAtual[ende++];

        // Pegar o número do primeiro bloco
        aux = "";
        for (int i = 0; i < TAM_BLOCO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo->numPrimeiroBloco = stoi(aux);

        if (subArqInfo->ehDiretorio == 'D') {
            Diretorio *subDir = new Diretorio();
            subDir->informacoes = subArqInfo;
            subDir->carrega(subArqInfo->numPrimeiroBloco);
            subDiretorio.push_back(subDir);
        } else {
            Arquivo *subArq = new Arquivo();
            subArq->informacoes = subArqInfo;
            subArq->carrega(subArqInfo->numPrimeiroBloco);
            subArquivo.push_back(subArq);
        }

        // Pegar os tempos
        for (time_t *tempo :
             { &subArqInfo->tempoCriado, &subArqInfo->tempoModificado,
               &subArqInfo->tempoAcesso })
            *tempo = pegaTempo(TAM_TEMPO);

        // Pegar o tamanho do arquivo (ou zeros se for diretório)
        aux = "";
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo->tamanho = stoi(aux);

        // Pegar o nome na heap
        int enderecoPonteiro = subArqInfo->ptNome;
        aux = "";
        pulaBloco();
        for (; discoAtual[enderecoPonteiro] != SEPARADOR;
             enderecoPonteiro++, pulaBloco())
            aux.push_back(discoAtual[enderecoPonteiro]);
        subArqInfo->nome = aux;

        subArqInfo->pai = this;
    }

    informacoes->espacoDesperdicado = limite - ende;
    espacoDesperdicadoTotal += informacoes->espacoDesperdicado;

    auto porNome = [](ArquivoGenerico *a1, ArquivoGenerico *a2) {
        return a1->informacoes->nome < a2->informacoes->nome;
    };

    sort(subDiretorio.begin(), subDiretorio.end(), porNome);
    sort(subArquivo.begin(), subArquivo.end(), porNome);
}

void Diretorio::salva() {
    int base, limite;
    int ende; // Endereço atual sendo lido
    int numBloco = informacoes->numPrimeiroBloco;

    auto alocaaux = [&ende, &numBloco, &base, &limite]() {
        if (ende == limite) {
            if (FAT.ponteiro[numBloco] == BLOCO_NULO) {
                // preciso alocar um novo bloco
                numBloco = FAT.alocaBloco(numBloco);
                if (numBloco == BLOCO_INVALIDO)
                    throw string("Memória insuficiente");
            }
            ende = blocoEmBaseLimite(numBloco, base, limite);
        }
    };

    auto escreveTempo = [&ende, &alocaaux, this](time_t tempo) {
        string aux;
        aux = to_string(tempo);
        if ((int) aux.size() != TAM_TEMPO)
            throw string("Tempo deveria ser igual a " + to_string(TAM_TEMPO));
        for (int i = 0; i < TAM_TEMPO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    if (DEBUG)
        cout << "Salvando o diretório " << informacoes->nome << " na posição "
             << ende << endl;

    // Escrever as entradas de arquivo
    queue<int> fila;
    for (ArquivoInfo *subArqInfo : subArquivoInfo) {
        // Escrever o ponteiro para o nome na heap
        alocaaux();
        fila.push(ende);
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = CHAR_NULO;
        }

        // A flag de arquivo ou diretório
        alocaaux();
        discoAtual[ende++] = (subArqInfo->ehDiretorio);

        // Escrever o número do primeiro bloco
        string aux = intParaString(subArqInfo->numPrimeiroBloco, TAM_BLOCO);
        for (int i = 0; i < TAM_BLOCO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }

        // Escrever os tempos
        for (time_t tempo :
             { subArqInfo->tempoCriado, subArqInfo->tempoModificado,
               subArqInfo->tempoAcesso })
            escreveTempo(tempo);

        // Escrever o tamanho do arquivo (ou zeros se for diretório)
        aux = intParaString(subArqInfo->tamanho, TAM_TAMANHO);
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    }

    alocaaux();
    discoAtual[ende++] = SEPARADOR; // para indicar o começo da heap

    // Escrever a heap
    int enderecoPonteiro;
    for (ArquivoInfo *subArqInfo : subArquivoInfo) {
        enderecoPonteiro = fila.front();
        fila.pop();

        // Voltar no ponteiro e fazer ele apontar para o começo do nome
        string aux = intParaString(ende, TAM_ENDERECO);
        for (int i = 0; i < TAM_ENDERECO; i++, enderecoPonteiro++) {
            if (enderecoPonteiro == limite) {
                numBloco = FAT.ponteiro[numBloco];
                enderecoPonteiro = blocoEmBaseLimite(numBloco, base, limite);
            }
            discoAtual[enderecoPonteiro] = aux[i];
        }

        // Escrevo o nome na heap
        for (int i = 0; i < (int) subArqInfo->nome.size(); i++, ende++) {
            alocaaux();
            discoAtual[ende] = subArqInfo->nome[i];
        }
        alocaaux();
        discoAtual[ende++] = SEPARADOR;
    }

    espacoDesperdicadoTotal -= informacoes->espacoDesperdicado;
    informacoes->espacoDesperdicado = limite - ende;
    espacoDesperdicadoTotal += informacoes->espacoDesperdicado;
    FAT.liberaBlocos(FAT.ponteiro[numBloco]);
}

void Diretorio::salvaTudo() {
    try {
        salva();
    } catch (string err) {
        cerr << "Diretorio::salvaTudo(): erro inesperado:\n" << err << endl;
    }

    for (Diretorio *subDir : subDiretorio) subDir->salvaTudo();
    for (Arquivo *subArq : subArquivo) subArq->salva();
}

void Diretorio::adiciona(Diretorio *dir) {
    auto porNome = [](ArquivoGenerico *arq, string nome) {
        return arq->informacoes->nome < nome;
    };

    dir->informacoes->pai = this;
    subArquivoInfo.push_back(dir->informacoes);

    auto iterDir = lower_bound(subDiretorio.begin(), subDiretorio.end(),
                               dir->informacoes->nome, porNome);

    subDiretorio.insert(iterDir, dir);

    informacoes->atualizaTempo(T_ACESSO + T_MODIFICADO);

    salva();
}

void Diretorio::adiciona(Arquivo *arq) {
    auto porNome = [](ArquivoGenerico *arq, string nome) {
        return arq->informacoes->nome < nome;
    };

    arq->informacoes->pai = this;
    subArquivoInfo.push_back(arq->informacoes);

    auto iterDir = lower_bound(subArquivo.begin(), subArquivo.end(),
                               arq->informacoes->nome, porNome);

    subArquivo.insert(iterDir, arq);

    informacoes->atualizaTempo(T_ACESSO + T_MODIFICADO);

    salva();
}

void Diretorio::remove(Diretorio *dir) {
    auto porNome = [](ArquivoGenerico *arq, string nome) {
        return arq->informacoes->nome < nome;
    };

    for (Diretorio *subDir : dir->subDiretorio){
        cout << "Removendo subdiretório " + subDir->informacoes->nome << endl;
        dir->remove(subDir);
    }

    for (Arquivo *subArq : dir->subArquivo){
        cout << "Removendo subarquivo " + subArq->informacoes->nome << endl;
        dir->remove(subArq);
    }

    auto it = subArquivoInfo.begin();
    for (; (*it) != dir->informacoes; it++)
        ;

    subArquivoInfo.erase(it);

    auto iterDir = lower_bound(subDiretorio.begin(), subDiretorio.end(),
                               dir->informacoes->nome, porNome);

    subDiretorio.erase(iterDir);

    FAT.liberaBlocos(dir->informacoes->numPrimeiroBloco);
    delete (dir);

    informacoes->atualizaTempo(T_ACESSO + T_MODIFICADO);

    salva();
}

void Diretorio::remove(Arquivo *arq) {
    auto porNome = [](ArquivoGenerico *arq, string nome) {
        return arq->informacoes->nome < nome;
    };

    auto it = subArquivoInfo.begin();
    for (; (*it) != arq->informacoes; it++)
        ;

    subArquivoInfo.erase(it);

    auto iterArq = lower_bound(subArquivo.begin(), subArquivo.end(),
                               arq->informacoes->nome, porNome);

    subArquivo.erase(iterArq);

    FAT.liberaBlocos(arq->informacoes->numPrimeiroBloco);
    delete (arq);

    informacoes->atualizaTempo(T_ACESSO + T_MODIFICADO);

    salva();
}

ArquivoGenerico *Diretorio::busca(string nomeArquivo) {
    auto comparaPorNome = [](ArquivoGenerico *arq, string nome) {
        return arq->informacoes->nome < nome;
    };

    auto iterDir = lower_bound(subDiretorio.begin(), subDiretorio.end(),
                               nomeArquivo, comparaPorNome);

    if (iterDir == subDiretorio.end() ||
        (*iterDir)->informacoes->nome != nomeArquivo) {
        auto iterArq = lower_bound(subArquivo.begin(), subArquivo.end(),
                                   nomeArquivo, comparaPorNome);

        if (iterArq == subArquivo.end() ||
            (*iterArq)->informacoes->nome != nomeArquivo)
            return nullptr;
        ;

        return *iterArq;
    }
    return *iterDir;
}

bool Diretorio::buscaAbaixo(string caminho, string nomeAtual) {
    bool retorno = false;
    string novoCaminho;

    ArquivoGenerico *arq = busca(nomeAtual);

    if (arq != nullptr) {
        cout << caminho << "/" << arq->informacoes->nome << endl;
        retorno = true;
    }

    for (Diretorio *dir : subDiretorio) {
        if (caminho.size() == 1)
            novoCaminho = caminho + dir->informacoes->nome;
        else
            novoCaminho = caminho + "/" + dir->informacoes->nome;
        retorno = dir->buscaAbaixo(novoCaminho, nomeAtual) || retorno;
    }
    return retorno;
}

void Diretorio::libera() {
    for (Diretorio *subDir : subDiretorio) {
        subDir->libera();
        delete subDir;
    }
    subDiretorio.clear();

    // for (ArquivoInfo *arqInfo : subArquivoInfo) delete arqInfo;
    subArquivoInfo.clear();

    for (Arquivo *arq : subArquivo) delete arq;
    subArquivo.clear();
}

Root::Root() { informacoes = new ArquivoInfo(); }

Root::~Root() { }

void Root::inicializa() {
    informacoes->nome = "/";
    informacoes->atualizaTempo(T_TODOS);
    informacoes->ehDiretorio = 'D';
    informacoes->ptNome = 0;
    informacoes->pai = nullptr;
    informacoes->tamanho = 0;
    informacoes->numPrimeiroBloco = 0;
    informacoes->espacoDesperdicado = 0; // isso é mudado no futuro, ao salvar
    Bitmap.livre[0] = OCUPADO;
    Bitmap.blocosLivres--;
    FAT.ponteiro[0] = BLOCO_NULO;
}

void Root::carrega(int numBloco) {
    int base, limite;
    int ende; // Endereço atual sendo lido
    informacoes->numPrimeiroBloco = numBloco;

    auto pulaBloco = [&ende, &numBloco, &base, &limite]() {
        if (ende == limite) {
            numBloco = FAT.ponteiro[numBloco];
            ende = blocoEmBaseLimite(numBloco, base, limite);
        }
    };

    auto pegaTempo = [&ende](int tam) {
        string aux;
        aux = "";
        for (int i = 0; i < tam; i++, ende++) aux.push_back(discoAtual[ende]);
        return (time_t) stoi(aux);
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    if (DEBUG) cout << "Carregando / do endereço " << ende << endl;

    // Root não tem pai
    informacoes->pai = nullptr;

    // Pegar os tempos criado, modificado e acessado
    for (time_t *tempo :
         { &informacoes->tempoCriado, &informacoes->tempoModificado,
           &informacoes->tempoAcesso })
        *tempo = pegaTempo(TAM_TEMPO);

    // Pegar o nome do diretório
    string aux = "";
    while (discoAtual[ende] != SEPARADOR) aux.push_back(discoAtual[ende++]);
    informacoes->nome = aux;

    ende++; // pula o SEPARADOR que indica o fim do nome

    while (discoAtual[ende] != SEPARADOR) {
        ArquivoInfo *subArqInfo = new ArquivoInfo();
        subArquivoInfo.push_back(subArqInfo);

        // Pegar o ponteiro para o nome na heap
        string aux = "";
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo->ptNome = stoi(aux);

        // Pegar a flag de arquivo ou diretório
        pulaBloco();
        subArqInfo->ehDiretorio = discoAtual[ende++];

        // Pegar o número do primeiro bloco
        aux = "";
        for (int i = 0; i < TAM_BLOCO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo->numPrimeiroBloco = stoi(aux);

        if (subArqInfo->ehDiretorio == 'D') {
            Diretorio *subDir = new Diretorio();
            subDir->informacoes = subArqInfo;
            subDir->carrega(subArqInfo->numPrimeiroBloco);
            subDiretorio.push_back(subDir);
        } else {
            Arquivo *subArq = new Arquivo();
            subArq->informacoes = subArqInfo;
            subArq->carrega(subArqInfo->numPrimeiroBloco);
            subArquivo.push_back(subArq);
        }

        // Pegar os tempos
        for (time_t *tempo :
             { &subArqInfo->tempoCriado, &subArqInfo->tempoModificado,
               &subArqInfo->tempoAcesso })
            *tempo = pegaTempo(TAM_TEMPO);

        // Pegar o tamanho do arquivo (ou zeros se for diretório)
        aux = "";
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo->tamanho = stoi(aux);

        // Pegar o nome na heap
        int enderecoPonteiro = subArqInfo->ptNome;
        aux = "";
        pulaBloco();
        for (; discoAtual[enderecoPonteiro] != SEPARADOR;
             enderecoPonteiro++, pulaBloco())
            aux.push_back(discoAtual[enderecoPonteiro]);
        subArqInfo->nome = aux;

        subArqInfo->pai = this;
    }

    auto porNome = [](ArquivoGenerico *a1, ArquivoGenerico *a2) {
        return a1->informacoes->nome < a2->informacoes->nome;
    };

    sort(subDiretorio.begin(), subDiretorio.end(), porNome);
    sort(subArquivo.begin(), subArquivo.end(), porNome);
}

void Root::salva() {
    int base, limite;
    int ende; // Endereço atual sendo lido
    int numBloco = 0;

    auto alocaaux = [&ende, &numBloco, &base, &limite]() {
        if (ende == limite) {
            if ((numBloco = FAT.ponteiro[numBloco]) == BLOCO_NULO) {
                // preciso alocar um novo bloco
                numBloco = FAT.alocaBloco(numBloco);
            }
            ende = blocoEmBaseLimite(numBloco, base, limite);
        }
    };

    auto escreveTempo = [&ende, &alocaaux, this](time_t tempo) {
        string aux;
        aux = to_string(tempo);
        if ((int) aux.size() != TAM_TEMPO)
            throw string("Tempo deveria ser igual a " + to_string(TAM_TEMPO));
        for (int i = 0; i < TAM_TEMPO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    // Escrever os tempos criado, modificado e acessado
    for (time_t tempo :
         { informacoes->tempoCriado, informacoes->tempoModificado,
           informacoes->tempoAcesso })
        escreveTempo(tempo);

    // Escrever o nome do diretório
    for (int i = 0; i < (int) informacoes->nome.size(); i++, ende++)
        discoAtual[ende] = informacoes->nome[i];
    discoAtual[ende++] = SEPARADOR;

    // Escrever as entradas de arquivo
    queue<int> fila;
    for (ArquivoInfo *subArqInfo : subArquivoInfo) {
        // Escrever o ponteiro para o nome na heap
        alocaaux();
        fila.push(ende);
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = CHAR_NULO;
        }

        // A flag de arquivo ou diretório
        alocaaux();
        discoAtual[ende++] = (subArqInfo->ehDiretorio);

        // Escrever o número do primeiro bloco
        string aux = intParaString(subArqInfo->numPrimeiroBloco, TAM_BLOCO);
        for (int i = 0; i < TAM_BLOCO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }

        // Escrever os tempos
        for (time_t tempo :
             { subArqInfo->tempoCriado, subArqInfo->tempoModificado,
               subArqInfo->tempoAcesso })
            escreveTempo(tempo);

        // Escrever o tamanho do arquivo (ou zeros se for diretório)
        aux = intParaString(subArqInfo->tamanho, TAM_TAMANHO);
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    }

    alocaaux();
    discoAtual[ende++] = SEPARADOR; // para indicar o começo da heap

    // Escrever a heap
    int enderecoPonteiro;
    for (ArquivoInfo *subArqInfo : subArquivoInfo) {
        enderecoPonteiro = fila.front();
        fila.pop();

        // Voltar no ponteiro e fazer ele apontar para o começo do nome
        string aux = intParaString(ende, TAM_ENDERECO);
        for (int i = 0; i < TAM_ENDERECO; i++, enderecoPonteiro++) {
            if (enderecoPonteiro == limite) {
                numBloco = FAT.ponteiro[numBloco];
                enderecoPonteiro = blocoEmBaseLimite(numBloco, base, limite);
            }
            discoAtual[enderecoPonteiro] = aux[i];
        }

        // Escrevo o nome na heap
        for (int i = 0; i < (int) subArqInfo->nome.size(); i++, ende++) {
            alocaaux();
            discoAtual[ende] = subArqInfo->nome[i];
        }
        alocaaux();
        discoAtual[ende++] = SEPARADOR;
    }

    espacoDesperdicadoTotal -= informacoes->espacoDesperdicado;
    informacoes->espacoDesperdicado = limite - ende;
    espacoDesperdicadoTotal += informacoes->espacoDesperdicado;
    FAT.liberaBlocos(FAT.ponteiro[numBloco]);
}

void FAT_t::inicializa() {
    for (int i = 0; i < NUM_BLOCOS; i++) ponteiro[i] = BLOCO_NULO;
}

void FAT_t::carrega(int numBloco) {
    string aux;
    for (int j = 0, ende = TAM_BITMAP; j < NUM_BLOCOS; j++) {
        aux = "";
        for (int k = 0; k < TAM_BLOCO; k++, ende++) {
            aux.push_back(discoAtual[ende]);
        }
        ponteiro[j] = stoi(aux);
    }
}

void FAT_t::salva() {
    string aux;
    int ende = TAM_BITMAP;

    if (DEBUG) cout << "FAT: salvando começando no endereço " << ende << endl;

    for (int j = 0; j < NUM_BLOCOS; j++) {
        aux = intParaString(ponteiro[j], TAM_BLOCO);
        for (int k = 0; k < TAM_BLOCO; k++, ende++) discoAtual[ende] = aux[k];
    }
    for (; ende < TAM_BITMAP + TAM_FAT; ende++) discoAtual[ende] = CHAR_NULO;

    if (DEBUG) cout << "FAT: último endereço salvo = " << ende - 1 << endl;
}

int FAT_t::alocaBloco() {
    try {
        int blocoLivre = Bitmap.pegaProxLivre();

        if (DEBUG)
            cout << "FAT_t::alocaBloco(): alocando o bloco = " << blocoLivre
                 << endl;

        Bitmap.livre[blocoLivre] = OCUPADO;
        Bitmap.blocosLivres--;
        ponteiro[blocoLivre] = BLOCO_NULO;

        return blocoLivre;
    } catch (string erro) {
        cerr << "Erro ao alocar um bloco de memória:\n" << erro << endl;
        return BLOCO_INVALIDO;
    }
    return BLOCO_INVALIDO;
}

int FAT_t::alocaBloco(int numPrimeiroBloco) {
    try {
        int bloco_livre = Bitmap.pegaProxLivre();

        cout << numPrimeiroBloco << endl;
        while (ponteiro[numPrimeiroBloco] != BLOCO_NULO)
            numPrimeiroBloco = ponteiro[numPrimeiroBloco];

        ponteiro[numPrimeiroBloco] = bloco_livre;
        Bitmap.livre[bloco_livre] = OCUPADO;
        Bitmap.blocosLivres--;
        ponteiro[bloco_livre] = BLOCO_NULO;

        return bloco_livre;
    } catch (string erro) {
        cerr << "Erro ao alocar um bloco de memória:\n" << erro << endl;
        return BLOCO_INVALIDO;
    }
    return BLOCO_INVALIDO;
}

void FAT_t::liberaBlocos(int numBloco) {
    int prox;

    while (numBloco != BLOCO_NULO) {

        if(DEBUG){
            cout << "FAT_t::liberaBlocos(): Liberando bloco " << numBloco << " ..." << endl;
        }

        prox = FAT.ponteiro[numBloco];
        Bitmap.livre[numBloco] = LIVRE;
        Bitmap.blocosLivres++;
        FAT.ponteiro[numBloco] = BLOCO_NULO;

        limpaBloco(numBloco);
        numBloco = prox;
    }
}

void Bitmap_t::inicializa() {
    for (int i = 0; i < NUM_BLOCOS; i++) livre[i] = LIVRE;
    proxLivre = 0;
    blocosLivres = NUM_BLOCOS;
}

void Bitmap_t::carrega(int numBloco) {
    blocosLivres = 0;
    for (int i = 0; i < NUM_BLOCOS; i++) {
        livre[i] = (bool) (discoAtual[i] - '0');
        if (livre[i]) blocosLivres++;
    }
    proxLivre = 1;
    while (livre[proxLivre] == OCUPADO && proxLivre != 0) {
        proxLivre++;
        proxLivre %= NUM_BLOCOS;
    }
}

void Bitmap_t::salva() {
    int ende = 0;

    if (DEBUG) cout << "Bitmap: salvando começando do endereço = 0" << endl;

    for (ende = 0; ende < NUM_BLOCOS; ende++)
        discoAtual[ende] = (char) livre[ende] + '0';
    for (; ende < TAM_BITMAP; ende++) discoAtual[ende] = CHAR_NULO;
    if (DEBUG)
        cout << "Bitmap: último endereço salvo = " << TAM_BITMAP - 1 << endl;
}

int Bitmap_t::pegaProxLivre() {
    int blocoInicial = proxLivre - 1;
    if (blocoInicial < 0) blocoInicial += NUM_BLOCOS;

    if (DEBUG)
        cout << "Bitmap_t::pegaProxLivre(): bloco inicial = " << blocoInicial
             << endl;

    while (livre[proxLivre] == OCUPADO && proxLivre != blocoInicial) {
        proxLivre++;
        proxLivre %= NUM_BLOCOS;
    }

    if (proxLivre == blocoInicial) throw string("O disco está cheio.");

    return proxLivre;
}
