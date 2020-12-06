#include "diretorio.hpp"

#include <algorithm>
#include <queue>

#include "bitmap.hpp"
#include "fat.hpp"

Root root;

Diretorio::Diretorio() : ArquivoGenerico() { qntDiretorios++; }

Diretorio::~Diretorio() { qntDiretorios--; }

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
        int auxNumBloco, auxBase, auxLimite;
        auxNumBloco = enderecoEmBloco(enderecoPonteiro);
        blocoEmBaseLimite(auxNumBloco, auxBase, auxLimite);
        aux = "";
        for (; discoAtual[enderecoPonteiro] != SEPARADOR;) {
            aux.push_back(discoAtual[enderecoPonteiro]);
            enderecoPonteiro++;
            if (enderecoPonteiro == auxLimite) {
                auxNumBloco = FAT.ponteiro[auxNumBloco];
                enderecoPonteiro =
                    blocoEmBaseLimite(auxNumBloco, auxBase, auxLimite);
            }
        }
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
        if (ende >= limite) {
            if (FAT.ponteiro[numBloco] == BLOCO_NULO) {
                // preciso alocar um novo bloco
                numBloco = FAT.alocaBloco(numBloco);
                if (numBloco == BLOCO_INVALIDO)
                    throw string("Memória insuficiente");
            } else {
                // pulo para o próximo bloco
                numBloco = FAT.ponteiro[numBloco];
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

        int auxBloco, auxBase, auxLimite;
        auxBloco = enderecoEmBloco(enderecoPonteiro);
        blocoEmBaseLimite(auxBloco, auxBase, auxLimite);

        // Voltar no ponteiro e fazer ele apontar para o começo do nome
        alocaaux();
        string aux = intParaString(ende, TAM_ENDERECO);
        for (int i = 0; i < TAM_ENDERECO; i++, enderecoPonteiro++) {
            if (enderecoPonteiro == auxLimite) {
                auxBloco = FAT.ponteiro[auxBloco];
                enderecoPonteiro =
                    blocoEmBaseLimite(auxBloco, auxBase, auxLimite);
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

    auto itDir = dir->subDiretorio.begin();
    auto endDir = dir->subDiretorio.end();
    while (itDir != endDir) {
        Diretorio *subDir = *itDir;
        cout << "Removendo subdiretório " + subDir->informacoes->nome << endl;
        dir->remove(subDir);
        /* atualizar o iterador para o fim é necessário */
        endDir = dir->subDiretorio.end();
    }

    auto itArq = dir->subArquivo.begin();
    auto endArq = dir->subArquivo.end();
    while (itArq != endArq) {
        Arquivo *subArq = *itArq;
        cout << "Removendo subarquivo " + subArq->informacoes->nome << endl;
        dir->remove(subArq);
        /* atualizar o iterador para o fim é necessário */
        endArq = dir->subArquivo.end();
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
        int auxNumBloco, auxBase, auxLimite;
        auxNumBloco = enderecoEmBloco(enderecoPonteiro);
        blocoEmBaseLimite(auxNumBloco, auxBase, auxLimite);
        aux = "";
        for (; discoAtual[enderecoPonteiro] != SEPARADOR;) {
            aux.push_back(discoAtual[enderecoPonteiro]);
            enderecoPonteiro++;
            if (enderecoPonteiro == auxLimite) {
                auxNumBloco = FAT.ponteiro[auxNumBloco];
                enderecoPonteiro =
                    blocoEmBaseLimite(auxNumBloco, auxBase, auxLimite);
            }
        }
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
        if (ende >= limite) {
            if (FAT.ponteiro[numBloco] == BLOCO_NULO) {
                // preciso alocar um novo bloco
                numBloco = FAT.alocaBloco(numBloco);
                if (numBloco == BLOCO_INVALIDO)
                    throw string("Memória insuficiente");
            } else {
                // pulo para o próximo bloco
                numBloco = FAT.ponteiro[numBloco];
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

        int auxBloco, auxBase, auxLimite;
        auxBloco = enderecoEmBloco(enderecoPonteiro);
        blocoEmBaseLimite(auxBloco, auxBase, auxLimite);

        // Voltar no ponteiro e fazer ele apontar para o começo do nome
        alocaaux();
        string aux = intParaString(ende, TAM_ENDERECO);
        for (int i = 0; i < TAM_ENDERECO; i++, enderecoPonteiro++) {
            if (enderecoPonteiro == auxLimite) {
                auxBloco = FAT.ponteiro[auxBloco];
                enderecoPonteiro =
                    blocoEmBaseLimite(auxBloco, auxBase, auxLimite);
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

