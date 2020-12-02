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
void limpaBloco(int numBloco){
    int ende, base, limite;
    ende = blocoEmBaseLimite(numBloco, base, limite);

    for(; ende < limite; ende++)
        discoAtual[ende] = CHAR_NULO;
}

void limpaBloco(int ende, int numBloco){
    int base, limite;
    blocoEmBaseLimite(numBloco, base, limite);

    for(; ende < limite; ende++)
        discoAtual[ende] = CHAR_NULO;
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
        cout << setw(2) << timeInfo->tm_mday << " " << intParaMes(timeInfo->tm_mon) << " "
             << setw(2) << setfill('0') << timeInfo->tm_hour << ":" << setw(2)
             << setfill('0') << timeInfo->tm_min << "  ";
    };

    cout << ehDiretorio << " ";
    cout << setw(5) << ((ehDiretorio == 'D') ? "-" : bytesFormatados(tamanho))
         << " ";
    // TEMPORARIO: depois deixar apenas o tempo de última modificação
    for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
        imprimeTempo(tempo);

    cout << nome << endl;
}

ArquivoGenerico *caminhoParaArquivo(string caminho) {
    Diretorio *atual = &root;
    ArquivoGenerico * retorn = nullptr;

    int i;
    string nomeAtual;

    i = 1; // ignoramos '/' no começo da string
    if (caminho[(int) caminho.size() - 1] == '/') caminho.pop_back();

    while (i < (int) caminho.size()) {
        nomeAtual = "";
        while (i < (int) caminho.size() && caminho[i++] != '/')
            nomeAtual.push_back(caminho[i]);

        if (i < (int) caminho.size()) {
            try {
                atual = (Diretorio *) atual->busca(nomeAtual);
            } catch (string exc) {
                cerr << "ERRO: " << exc << endl;
                cerr << "Retornando arquivo nulo!" << endl;
                return nullptr;
            }
        }
    }

    try {
        retorn = atual->busca(nomeAtual);
    } catch (string exc) {
        cerr << "ERRO: " << exc << endl;
        cerr << "Retornando arquivo nulo!" << endl;
        return nullptr;
    }

    return retorn;
}

void ArquivoGenerico::carrega(int numBloco) { }

void ArquivoGenerico::salva() { }

void Arquivo::carrega(int numBloco) {
    // Base e limite para o bloco que estamos atualmente carregando
    int base, limite;
    int ende; // Endereço atual sendo lido

    // auto pegaTempo = [&ende](int tam) {
    // string aux;
    // aux = "";
    // for (int i = 0; i < tam; i++, ende++) aux.push_back(discoAtual[ende]);
    // return (time_t) stoi(aux);
    // };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    ende += 2; // Pular o '\\' e o 'A'

    // Pegar os tempos criado, modificado e acessado
    // for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
    // *tempo = pegaTempo(TAM_TEMPO);

    // Pegar o tamanho do arquivo
    // string aux = "";
    // for (int i = 0; i < TAM_TAMANHO; i++, ende++)
    // aux.push_back(discoAtual[ende]);
    // tamanho = stoi(aux);

    // Pegar o nome
    // aux = "";
    // while (discoAtual[ende] != '|') aux.push_back(discoAtual[ende++]);
    // nome = aux;

    conteudo = "";
    while (numBloco != BLOCO_NULO) {
        for (; ende < limite; ende++) conteudo.push_back(discoAtual[ende++]);

        numBloco = FAT.ponteiro[numBloco]; // próximo bloco
        ende = blocoEmBaseLimite(numBloco, base, limite);
    }
}

void Arquivo::salva() {
    int base, limite;
    int ende; // Endereço atual sendo lido
    int numBloco = informacoes->numPrimeiroBloco;

    // auto escreveTempo = [&ende, this](time_t tempo) {
    // string aux;
    // aux = to_string(tempo);
    // if ((int) aux.size() != TAM_TEMPO)
    // throw "Tempo deveria ser igual a " + to_string(TAM_TEMPO);
    // for (int i = 0; i < TAM_TEMPO; i++, ende++) discoAtual[ende] = aux[i];
    // };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    discoAtual[ende++] = '\\';

    discoAtual[ende++] = 'A';

    // Escrever os tempos criado, modificado e acessado
    // for (time_t tempo : { tempoCriado, tempoModificado, tempoAcesso })
    // escreveTempo(tempo);

    // Escrever o tamanho do arquivo
    // string aux = intParaString(tamanho, TAM_TAMANHO);

    // for (int i = 0; i < TAM_TAMANHO; i++, ende++) discoAtual[ende] = aux[i];

    // Escrever o nome do arquivo
    // for (int i = 0; i < (int) nome.size(); i++, ende++)
    // discoAtual[ende] = nome[i];
    // discoAtual[ende++] = '|';

    int i = 0; // posicao atual do conteudo
    while (numBloco != BLOCO_NULO) {
        for (; i < (int) conteudo.size() && ende < limite; i++, ende++)
            discoAtual[ende] = conteudo[i];

        // se o conteudo acabou antes do limite do bloco, preenchemos com 'nulo'
        while (ende < limite) discoAtual[ende] = CHAR_NULO;

        /* TODO: Preciso alocar mais blocos? <23-11-20, Lucas> */
        numBloco = FAT.ponteiro[numBloco]; // próximo bloco
        ende = blocoEmBaseLimite(numBloco, base, limite);
    }
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

    ende += 2; // Pular o '\\' e 'D'

    // Pegar os tempos criado, modificado e acessado
    // for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
    // *tempo = pegaTempo(TAM_TEMPO);

    // Pegar o nome do diretório
    // string aux = "";
    // while (discoAtual[ende] != '|') aux.push_back(discoAtual[ende++]);
    // nome = aux;

    // Pegar as entradas de arquivo
    for (ArquivoInfo &subArqInfo : subArquivoInfo) {
        // Pegar o ponteiro para o nome na heap
        string aux = "";
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo.ptNome = stoi(aux);

        // Pegar a flag de arquivo ou diretório
        pulaBloco();
        subArqInfo.ehDiretorio = discoAtual[ende++];

        // Pegar o número do primeiro bloco
        aux = "";
        for (int i = 0; i < TAM_BLOCO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo.numPrimeiroBloco = stoi(aux);

        if (subArqInfo.ehDiretorio == 'D') {
            Diretorio *subDir = new Diretorio();
            subDir->carrega(subArqInfo.numPrimeiroBloco);
            subDir->informacoes = &subArqInfo;
            subDiretorio.push_back(subDir);
        } else {
            Arquivo *subArq = new Arquivo();
            subArq->carrega(subArqInfo.numPrimeiroBloco);
            subArq->informacoes = &subArqInfo;
            subArquivo.push_back(subArq);
        }

        // Pegar os tempos
        for (time_t *tempo :
             { &subArqInfo.tempoCriado, &subArqInfo.tempoModificado,
               &subArqInfo.tempoAcesso })
            *tempo = pegaTempo(TAM_TEMPO);

        // Pegar o tamanho do arquivo (ou zeros se for diretório)
        aux = "";
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo.tamanho = stoi(aux);

        // Pegar o nome na heap
        int enderecoPonteiro = subArqInfo.ptNome;
        aux = "";
        pulaBloco();
        for (; discoAtual[enderecoPonteiro] != '|';
             enderecoPonteiro++, pulaBloco())
            aux.push_back(discoAtual[enderecoPonteiro]);
        subArqInfo.nome = stoi(aux);

        subArqInfo.pai = this;
    }

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
            throw "Tempo deveria ser igual a " + to_string(TAM_TEMPO);
        for (int i = 0; i < TAM_TEMPO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    discoAtual[ende++] = '\\';
    discoAtual[ende++] = 'D';

    // Escrever os tempos criado, modificado e acessado
    // for (time_t tempo : { tempoCriado, tempoModificado, tempoAcesso })
    // escreveTempo(tempo);

    // Escrever o nome do diretório
    // for (int i = 0; i < (int) nome.size(); i++, ende++)
    // discoAtual[ende] = nome[i];
    // discoAtual[ende++] = '|';

    // Escrever as entradas de arquivo
    queue<int> fila;
    for (ArquivoInfo &subArqInfo : subArquivoInfo) {
        // Escrever o ponteiro para o nome na heap
        alocaaux();
        fila.push(ende);
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = '0';
        }

        // A flag de arquivo ou diretório
        alocaaux();
        discoAtual[ende++] = (subArqInfo.ehDiretorio);

        // Escrever o número do primeiro bloco
        string aux = intParaString(subArqInfo.numPrimeiroBloco, NUM_BLOCOS);
        for (int i = 0; i < NUM_BLOCOS; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }

        // Escrever os tempos
        for (time_t tempo :
             { subArqInfo.tempoCriado, subArqInfo.tempoModificado,
               subArqInfo.tempoAcesso })
            escreveTempo(tempo);

        // Escrever o tamanho do arquivo (ou zeros se for diretório)
        aux = intParaString(subArqInfo.tamanho, TAM_TAMANHO);
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    }

    // Escrever a heap
    int enderecoPonteiro;
    for (ArquivoInfo &subArqInfo : subArquivoInfo) {
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
        for (int i = 0; i < (int) subArqInfo.nome.size(); i++, ende++) {
            alocaaux();
            discoAtual[ende] = subArqInfo.nome[i];
        }
        alocaaux();
        discoAtual[ende++] = '|';
    }
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
            throw "Arquivo " + nomeArquivo + " não encontrado.";

        return *iterArq;
    }
    return *iterDir;
}

bool Diretorio::buscaAbaixo(string caminho, string nomeAtual){
    bool retorno = false;

    try{
        ArquivoGenerico * arq = busca(nomeAtual);
        cout << caminho << "/" << arq->informacoes->nome << endl;
        retorno = true;
    }
    catch(string erro) { }

    for(Diretorio * dir : subDiretorio)
        retorno = retorno || dir->buscaAbaixo(caminho + "/" + 
        dir->informacoes->nome, nomeAtual);
}

void Root::inicializa() {
    informacoes.nome = "/";
    informacoes.atualizaTempo(T_TODOS);
    informacoes.ehDiretorio = 'D';
    informacoes.ptNome = 0;
    informacoes.pai = nullptr;
    informacoes.tamanho = 0;
    informacoes.numPrimeiroBloco = 0;
}

void Root::carrega(int numBloco) {
    int base, limite;
    int ende; // Endereço atual sendo lido
    numBloco = 0;

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

    ende += 2; // Pular o '\\' e 'D'

    // Pegar os tempos criado, modificado e acessado
    for (time_t *tempo :
         { &informacoes.tempoCriado, &informacoes.tempoModificado,
           &informacoes.tempoAcesso })
        *tempo = pegaTempo(TAM_TEMPO);

    // Pegar o nome do diretório
    string aux = "";
    while (discoAtual[ende] != '|') aux.push_back(discoAtual[ende++]);
    informacoes.nome = aux;

    // Pegar as entradas de arquivo
    for (ArquivoInfo &subArqInfo : subArquivoInfo) {
        // Pegar o ponteiro para o nome na heap
        string aux = "";
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo.ptNome = stoi(aux);

        // Pegar a flag de arquivo ou diretório
        pulaBloco();
        subArqInfo.ehDiretorio = discoAtual[ende++];

        // Pegar o número do primeiro bloco
        aux = "";
        for (int i = 0; i < TAM_BLOCO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo.numPrimeiroBloco = stoi(aux);

        if (subArqInfo.ehDiretorio == 'D') {
            Diretorio *subDir = new Diretorio();
            subDir->carrega(subArqInfo.numPrimeiroBloco);
            subDir->informacoes = &subArqInfo;
            subDiretorio.push_back(subDir);
        } else {
            Arquivo *subArq = new Arquivo();
            subArq->carrega(subArqInfo.numPrimeiroBloco);
            subArq->informacoes = &subArqInfo;
            subArquivo.push_back(subArq);
        }

        // Pegar os tempos
        for (time_t *tempo :
             { &subArqInfo.tempoCriado, &subArqInfo.tempoModificado,
               &subArqInfo.tempoAcesso })
            *tempo = pegaTempo(TAM_TEMPO);

        // Pegar o tamanho do arquivo (ou zeros se for diretório)
        aux = "";
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subArqInfo.tamanho = stoi(aux);

        // Pegar o nome na heap
        int enderecoPonteiro = subArqInfo.ptNome;
        aux = "";
        pulaBloco();
        for (; discoAtual[enderecoPonteiro] != '|';
             enderecoPonteiro++, pulaBloco())
            aux.push_back(discoAtual[enderecoPonteiro]);
        subArqInfo.nome = stoi(aux);

        subArqInfo.pai = this;
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
            throw "Tempo deveria ser igual a " + to_string(TAM_TEMPO);
        for (int i = 0; i < TAM_TEMPO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    cout << "Root: ende inicial = " << ende << endl;

    discoAtual[ende++] = '\\';
    discoAtual[ende++] = 'D';

    // Escrever os tempos criado, modificado e acessado
    for (time_t tempo : { informacoes.tempoCriado, informacoes.tempoModificado,
                          informacoes.tempoAcesso })
        escreveTempo(tempo);

    // Escrever o nome do diretório
    for (int i = 0; i < (int) informacoes.nome.size(); i++, ende++)
        discoAtual[ende] = informacoes.nome[i];
    discoAtual[ende++] = '|';

    // Escrever as entradas de arquivo
    queue<int> fila;
    for (ArquivoInfo &subArqInfo : subArquivoInfo) {
        // Escrever o ponteiro para o nome na heap
        alocaaux();
        fila.push(ende);
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = '0';
        }

        // A flag de arquivo ou diretório
        alocaaux();
        discoAtual[ende++] = (subArqInfo.ehDiretorio);

        // Escrever o número do primeiro bloco
        string aux = intParaString(subArqInfo.numPrimeiroBloco, NUM_BLOCOS);
        for (int i = 0; i < NUM_BLOCOS; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }

        // Escrever os tempos
        for (time_t tempo :
             { subArqInfo.tempoCriado, subArqInfo.tempoModificado,
               subArqInfo.tempoAcesso })
            escreveTempo(tempo);

        // Escrever o tamanho do arquivo (ou zeros se for diretório)
        aux = intParaString(subArqInfo.tamanho, TAM_TAMANHO);
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    }

    // Escrever a heap
    int enderecoPonteiro;
    for (ArquivoInfo &subArqInfo : subArquivoInfo) {
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
        for (int i = 0; i < (int) subArqInfo.nome.size(); i++, ende++) {
            alocaaux();
            discoAtual[ende] = subArqInfo.nome[i];
        }
        alocaaux();
        discoAtual[ende++] = '|';
    }

    cout << "Root: ende final = " << ende << endl;
}

void FAT_t::inicializa() {
    for (int i = 0; i < NUM_BLOCOS; i++) ponteiro[i] = BLOCO_NULO;
}

void FAT_t::carrega(int numBloco) {
    string aux;
    for (int j = 0, ende = TAM_BITMAP; j < NUM_BLOCOS; j++) {
        aux = "";
        for (int k = 0; k < TAM_BLOCO; k++, ende++)
            aux.push_back(discoAtual[ende]);
        ponteiro[j] = stoi(aux);
    }
}

void FAT_t::salva() {
    string aux;
    int ende = TAM_BITMAP;
    cout << "FAT: ende inicial = " << ende << endl;
    for (int j = 0; j < NUM_BLOCOS; j++) {
        aux = intParaString(ponteiro[j], TAM_BLOCO);
        for (int k = 0; k < TAM_BLOCO; k++, ende++) discoAtual[ende] = aux[k];
    }
    for (; ende < TAM_BITMAP + TAM_FAT; ende++) discoAtual[ende] = CHAR_NULO;
    cout << "FAT: ende final = " << ende - 1 << endl;
}

int FAT_t::alocaBloco(int numPrimeiroBloco) {
    try {
        int bloco_livre = Bitmap.pegaProxLivre();

        while (ponteiro[numPrimeiroBloco] != BLOCO_NULO)
            numPrimeiroBloco = ponteiro[numPrimeiroBloco];

        ponteiro[numPrimeiroBloco] = bloco_livre;
        Bitmap.livre[bloco_livre] = false;
        Bitmap.blocosLivres--;
        ponteiro[bloco_livre] = BLOCO_NULO;

        return bloco_livre;
    } catch (string erro) {
        cerr << "Erro ao alocar um bloco de memória:\n" << erro << endl;
        return -1;
    }
    return -1;
}

void FAT_t::liberaBlocos(int numBloco){
    int prox;

    while(numBloco != BLOCO_NULO){

        prox = FAT.ponteiro[numBloco];
        Bitmap.livre[numBloco] = LIVRE; 
        FAT.ponteiro[numBloco] = BLOCO_NULO;

        limpaBloco(numBloco);
        numBloco = prox;
    }
}

void Bitmap_t::inicializa() {
    for (int i = 0; i < NUM_BLOCOS; i++) livre[i] = LIVRE;
}

void Bitmap_t::carrega(int numBloco) {
    blocosLivres = 0;
    for (int i = 0; i < NUM_BLOCOS; i++) {
        livre[i] = (bool) discoAtual[i] - '0';
        if (livre[i]) blocosLivres++;
    }
}

void Bitmap_t::salva() {
    int ende = 0;
    cout << "Bitmap: ende inicial = 0" << endl;
    for (ende = 0; ende < NUM_BLOCOS; ende++)
        discoAtual[ende] = (char) livre[ende] + '0';
    for (; ende < TAM_BITMAP; ende++) discoAtual[ende] = CHAR_NULO;
    cout << "Bitmap: ende final = " << TAM_BITMAP - 1 << endl;
}

int Bitmap_t::pegaProxLivre() {
    int blocoInicial = proxLivre - 1;
    if (blocoInicial < 0) blocoInicial += NUM_BLOCOS;

    while (livre[proxLivre] == false && proxLivre != blocoInicial) {
        proxLivre++;
        proxLivre %= NUM_BLOCOS;
    }

    if (proxLivre == blocoInicial) throw "O disco está cheio.";

    return proxLivre;
}
