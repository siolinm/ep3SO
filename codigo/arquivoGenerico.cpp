#include "arquivoGenerico.hpp"

#include <iomanip>

#include "diretorio.hpp"

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
    cout << setw(8) << setfill(' ')
         << ((ehDiretorio == 'D') ? "--------" : to_string(tamanho)) << " ";
    imprimeTempo(&tempoModificado);

    cout << nome << endl;
}

ArquivoGenerico::~ArquivoGenerico() { delete (informacoes); }

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
