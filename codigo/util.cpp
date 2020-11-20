#include "util.hpp"

#include <exception>
#include <fstream>
#include <string>
#include <vector>

string discoAtual;
FAT_t FAT;
Bitmap_t Bitmap;

unsigned int blocoEmEndereco(unsigned int numBloco) {
    return TAM_BITMAP + TAM_FAT + numBloco * 4000;
}

unsigned int enderecoEmBloco(unsigned int endereco) {
    return (endereco - TAM_BITMAP - TAM_FAT) / 4000;
}

unsigned int blocoEmBaseLimite(unsigned int numBloco, unsigned int &base,
                               unsigned int &limite) {
    unsigned int end = blocoEmEndereco(numBloco);
    base = end;
    limite = end + 4000;
}

void ArquivoGenerico::carrega(unsigned int numBloco) { }

void ArquivoGenerico::salva(unsigned int numBloco) { }

void Arquivo::carrega(unsigned int numBloco) {
    // Base e limite para o bloco que estamos atualmente carregando
    unsigned int base, limite;
    unsigned int ende; // Endereço atual sendo lido

    auto faux = [&ende](int tam) {
        string aux;
        aux = "";
        for (int i = 0; i < tam; i++, ende++) aux.push_back(discoAtual[ende]);
        return (time_t) stoi(aux);
    };

    blocoEmBaseLimite(numBloco, base, limite);
    ende = base;

    // Pegar os tempos criado, modificado e acessado
    for (time_t *tempo : { &tempoCriado, &tempoModificado, &tempoAcesso })
        *tempo = faux(TAM_TEMPO);

    // Pegar o nome
    string aux = "";
    while (discoAtual[ende] != '|') aux.push_back(discoAtual[ende++]);
    nome = aux;

    conteudo = "";
    while (numBloco != BLOCO_NULO) {
        for (; ende < limite; ende++) conteudo.push_back(discoAtual[ende++]);

        numBloco = FAT.ponteiro[numBloco]; // próximo bloco
        blocoEmBaseLimite(numBloco, base, limite);
        ende = base;
    }
}

void Arquivo::salva(unsigned int numBloco) {
    unsigned int base, limite;
    unsigned int ende; // Endereço atual sendo lido
    string aux;

    auto faux = [&ende, this](time_t tempo) {
        string aux;
        aux = to_string(tempo);
        if (aux.size() != TAM_TEMPO)
            throw "Tempo deveria ser igual a " + to_string(TAM_TEMPO);
        for (int i = 0; i < TAM_TEMPO; i++, ende++) discoAtual[ende] = aux[i];
    };

    blocoEmBaseLimite(numBloco, base, limite);
    ende = base;

    // Escrever os tempos criado, modificado e acessado
    for (time_t tempo : { tempoCriado, tempoModificado, tempoAcesso })
        faux(tempo);

    // Escrever o nome do arquivo
    for (int i = 0; i < nome.size(); i++, ende++) discoAtual[ende] = nome[i];
    discoAtual[ende++] = '|';

    int i = 0; // posicao atual do conteudo
    while (numBloco != BLOCO_NULO) {
        for (; i < conteudo.size() && ende < limite; i++, ende++)
            discoAtual[ende] = conteudo[i];

        // se o conteudo acabou antes do limite do bloco, preenchemos com 'nulo'
        while (ende < limite) discoAtual[ende] = 0;

        numBloco = FAT.ponteiro[numBloco]; // próximo bloco
        blocoEmBaseLimite(numBloco, base, limite);
        ende = base;
    }
}

void Diretorio::carrega(unsigned int numBloco) { }

void Diretorio::salva(unsigned int numBloco) { }

void Bitmap_t::carrega(unsigned int numBloco) {
    for (int i = 0; i < TAM_BITMAP; i++) cheio[i] = (bool) discoAtual[i] - '0';
}

void Bitmap_t::salva(unsigned int numBloco) {
    for (int i = 0; i < TAM_BITMAP; i++) discoAtual[i] = (char) cheio[i] + '0';
}
