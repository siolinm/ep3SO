#include "util.hpp"

#include <algorithm>
#include <exception>
#include <fstream>
#include <queue>
#include <string>
#include <vector>

string discoAtual;
FAT_t FAT;
Bitmap_t Bitmap;

int blocoEmEndereco(int numBloco) {
    return TAM_BITMAP + TAM_FAT + numBloco * 4000;
}

int enderecoEmBloco(int endereco) {
    return (endereco - TAM_BITMAP - TAM_FAT) / 4000;
}

int blocoEmBaseLimite(int numBloco, int &base, int &limite) {
    int end = blocoEmEndereco(numBloco);
    base = end;
    limite = end + 4000;
    return end;
}

string intParaString(int inteiro, int tamanho) {
    string ret;
    ret = to_string(inteiro);
    reverse(ret.begin(), ret.end());
    while (ret.size() < TAM_ENDERECO) ret.push_back('0');
    reverse(ret.begin(), ret.end());
    return ret;
}

void ArquivoGenerico::carrega(int numBloco) { }

void ArquivoGenerico::salva() { }

void Arquivo::carrega(int numBloco) {
    // Base e limite para o bloco que estamos atualmente carregando
    int base, limite;
    int ende; // Endereço atual sendo lido

    auto pegaTempo = [&ende](int tam) {
        string aux;
        aux = "";
        for (int i = 0; i < tam; i++, ende++) aux.push_back(discoAtual[ende]);
        return (time_t) stoi(aux);
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    numPrimeiroBloco = enderecoEmBloco(ende);

    ende += 2; // Pular o '\\' e o 'A'

    // Pegar os tempos criado, modificado e acessado
    for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
        *tempo = pegaTempo(TAM_TEMPO);

    // Pegar o tamanho do arquivo
    string aux = "";
    for (int i = 0; i < TAM_TAMANHO; i++, ende++)
        aux.push_back(discoAtual[ende]);
    tamanho = stoi(aux);

    // Pegar o nome
    aux = "";
    while (discoAtual[ende] != '|') aux.push_back(discoAtual[ende++]);
    nome = aux;

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
    int numBloco = numPrimeiroBloco;

    auto escreveTempo = [&ende, this](time_t tempo) {
        string aux;
        aux = to_string(tempo);
        if ((int) aux.size() != TAM_TEMPO)
            throw "Tempo deveria ser igual a " + to_string(TAM_TEMPO);
        for (int i = 0; i < TAM_TEMPO; i++, ende++) discoAtual[ende] = aux[i];
    };

    ende = blocoEmBaseLimite(numBloco, base, limite);

    discoAtual[ende++] = '\\';

    discoAtual[ende++] = 'A';

    // Escrever os tempos criado, modificado e acessado
    for (time_t tempo : { tempoCriado, tempoModificado, tempoAcesso })
        escreveTempo(tempo);

    // Escrever o tamanho do arquivo
    string aux = intParaString(tamanho, TAM_TAMANHO);

    for (int i = 0; i < TAM_TAMANHO; i++, ende++) discoAtual[ende] = aux[i];

    // Escrever o nome do arquivo
    for (int i = 0; i < (int) nome.size(); i++, ende++)
        discoAtual[ende] = nome[i];
    discoAtual[ende++] = '|';

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

    numPrimeiroBloco = enderecoEmBloco(ende);

    ende += 2; // Pular o '\\' e 'D'

    // Pegar os tempos criado, modificado e acessado
    for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
        *tempo = pegaTempo(TAM_TEMPO);

    // Pegar o nome do diretório
    string aux = "";
    while (discoAtual[ende] != '|') aux.push_back(discoAtual[ende++]);
    nome = aux;

    // Pegar as entradas de arquivo
    for (ArquivoInfo &subarq : subArquivo) {
        // Pegar o ponteiro para o nome na heap
        string aux = "";
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subarq.ptNome = stoi(aux);

        // Pegar a flag de arquivo ou diretório
        pulaBloco();
        subarq.ehDiretorio = discoAtual[ende++];

        // Pegar os tempos
        for (time_t *tempo : { &subarq.tempoCriado, &subarq.tempoModificado,
                               &subarq.tempoAcesso })
            *tempo = pegaTempo(TAM_TEMPO);

        // Pegar o tamanho do arquivo (ou zeros se for diretório)
        aux = "";
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            pulaBloco();
            aux.push_back(discoAtual[ende]);
        }
        subarq.tamanho = stoi(aux);

        // Pegar o nome na heap
        int enderecoPonteiro = subarq.ptNome;
        aux = "";
        pulaBloco();
        for (; discoAtual[enderecoPonteiro] != '|';
             enderecoPonteiro++, pulaBloco())
            aux.push_back(discoAtual[enderecoPonteiro]);
        subarq.nome = stoi(aux);
    }
}

void Diretorio::salva() {
    int base, limite;
    int ende; // Endereço atual sendo lido
    int numBloco = numPrimeiroBloco;

    auto alocaaux = [&ende, &numBloco, &base, &limite]() {
        if (ende == limite) {
            if ((numBloco = FAT.ponteiro[numBloco]) == BLOCO_NULO) {
                // preciso alocar um novo bloco
                // numBloco = aloca();
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
    for (time_t tempo : { tempoCriado, tempoModificado, tempoAcesso })
        escreveTempo(tempo);

    // Escrever o nome do diretório
    for (int i = 0; i < (int) nome.size(); i++, ende++)
        discoAtual[ende] = nome[i];
    discoAtual[ende++] = '|';

    // Escrever as entradas de arquivo
    queue<int> fila;
    for (ArquivoInfo &subarq : subArquivo) {
        // Escrever o ponteiro para o nome na heap
        alocaaux();
        fila.push(ende);
        for (int i = 0; i < TAM_ENDERECO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = '0';
        }

        // A flag de arquivo ou diretório
        alocaaux();
        discoAtual[ende++] = (subarq.ehDiretorio);

        // Escrever os tempos
        for (time_t tempo :
             { subarq.tempoCriado, subarq.tempoModificado, subarq.tempoAcesso })
            escreveTempo(tempo);

        // Escrever o tamanho do arquivo (ou zeros se for diretório)
        string aux = intParaString(subarq.tamanho, TAM_TAMANHO);
        for (int i = 0; i < TAM_TAMANHO; i++, ende++) {
            alocaaux();
            discoAtual[ende] = aux[i];
        }
    }

    // Escrever a heap
    int enderecoPonteiro;
    for (ArquivoInfo &subarq : subArquivo) {
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
        for (int i = 0; i < (int) subarq.nome.size(); i++, ende++) {
            alocaaux();
            discoAtual[ende] = subarq.nome[i];
        }
        alocaaux();
        discoAtual[ende++] = '|';
    }
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
    for (int j = 0, ende = TAM_BITMAP; j < NUM_BLOCOS; j++) {
        aux = intParaString(ponteiro[j], TAM_BLOCO);
        for (int k = 0; k < TAM_BLOCO; k++, ende++) discoAtual[ende] = aux[k];
    }
}

int FAT_t::alocaBloco(int numPrimeiroBloco) {
    try {
        int bloco_livre = Bitmap.pegaProxLivre();

        while (ponteiro[numPrimeiroBloco] != BLOCO_NULO)
            numPrimeiroBloco = ponteiro[numPrimeiroBloco];

        ponteiro[numPrimeiroBloco] = bloco_livre;
        Bitmap.cheio[bloco_livre] = true;
        Bitmap.blocosLivres--;
        ponteiro[bloco_livre] = BLOCO_NULO;

        return bloco_livre;
    } catch (string erro) {
        cerr << "Erro ao alocar um bloco de memória:\n" << erro << endl;
        return -1;
    }
    return -1;
}

void Bitmap_t::carrega(int numBloco) {
    blocosLivres = 0;
    for (int i = 0; i < TAM_BITMAP; i++) {
        cheio[i] = (bool) discoAtual[i] - '0';
        if (!cheio[i]) blocosLivres++;
    }
}

void Bitmap_t::salva() {
    for (int i = 0; i < TAM_BITMAP; i++) discoAtual[i] = (char) cheio[i] + '0';
}

int Bitmap_t::pegaProxLivre() {
    int blocoInicial = proxLivre - 1;
    if (blocoInicial < 0) blocoInicial += TAM_BITMAP;

    while (cheio[proxLivre] == true && proxLivre != blocoInicial) {
        proxLivre++;
        proxLivre %= TAM_BITMAP;
    }

    if (proxLivre == blocoInicial) throw "O disco está cheio.";

    return proxLivre;
}
