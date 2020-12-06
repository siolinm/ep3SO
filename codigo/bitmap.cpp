#include "bitmap.hpp"

Bitmap_t Bitmap;

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
