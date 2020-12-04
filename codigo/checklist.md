- [x] Modificar como salva o dir:
    - [x] Tirar metadados;
    - [x] Criar uma árvore de ponteiros;
    - [x] Vincular a parte de alocação.
- [x] Criar a função que dado um path, dê o obj do arquivo/diretorio
- [x] Terminar o mount (para criar a arvore de dir) (e implementar caso o arquivo n exista: criar novo sistema)
    - [x] Ver como salvar o /
    - [x] Tratar o / como corner
    - [x] Testar se a árvore está montando (precisa implementar a criação de arquivos primeiro)
- [x] Consertar o bitmap e a fat;
- [ ] Implementar os demais comandos:
    - [x] mount
    - [x] cp
    - [x] mkdir
    - [x] rmdir
    - [x] cat
    - [x] touch
    - [x] rm
    - [x] ls
    - [x] find
    - [ ] df
    - [x] umount
    - [x] sai
- [x] Dar free em tudo
- [x] Lembrar de mudar o tempo de acesso e modificação
  - [x] Quando implementar a função remove fazer isso automaticamente
  - [x] Quando implementar a função adiciona fazer isso automaticamente
- [ ] Checar por estourar o limite de memória (vai mudar o cp e comandos que alteram o diretório)
- [x] Decrementar a quantidade de arquivos e diretórios (provavelmente no destrutor)

- [ ] Tarefas se sobrar tempo:
    - [ ] usar a bib gnu history

- [ ] Erros encontrados
    - [x] diretório dentro de diretório
    - [x] bitmap não alocando
    - [x] carrega não está funcionando (só carrega a root)


| Comandos | Acesso | Modificação | Criação | Onde |
| :--: | :--: | :--: | :--: | :--: |
| cp | X | X |  | P |
| mkdir | X | X |  | P |
| rmdir | X | X |  | P |
| cat |  |  |  | |
| touch | X | X |  | A(P*) |
| rm | X | X |  | P |
| ls |   |   |   |  |
| find |   |   |   |   |
