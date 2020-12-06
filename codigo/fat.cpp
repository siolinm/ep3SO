#include "fat.hpp"

#include "bitmap.hpp"
#include "util.hpp"

FAT_t FAT;

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
        int blocoLivre = Bitmap.pegaProxLivre();

        if (DEBUG)
            cout << "FAT_t::alocaBloco(): alocando o bloco = " << blocoLivre
                 << endl;

        while (ponteiro[numPrimeiroBloco] != BLOCO_NULO)
            numPrimeiroBloco = ponteiro[numPrimeiroBloco];

        ponteiro[numPrimeiroBloco] = blocoLivre;
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

void FAT_t::liberaBlocos(int numBloco) {
    int prox;

    while (numBloco != BLOCO_NULO) {
        if (DEBUG) {
            cout << "FAT_t::liberaBlocos(): Liberando bloco " << numBloco
                 << " ..." << endl;
        }

        prox = FAT.ponteiro[numBloco];
        Bitmap.livre[numBloco] = LIVRE;
        Bitmap.blocosLivres++;
        FAT.ponteiro[numBloco] = BLOCO_NULO;

        limpaBloco(numBloco);
        numBloco = prox;
    }
}
