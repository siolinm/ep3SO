#include "arquivo.hpp"

#include "fat.hpp"

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

Arquivo::~Arquivo() { qntArquivos--; }

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
