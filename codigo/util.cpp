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
