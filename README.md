# Escalonamento de Tarefas Críticas de Voo

Simulador de escalonamento de tarefas periódicas de tempo real, comparando os
algoritmos Rate-Monotonic (RATE) e Earliest Deadline First (EDF).

## Arquivos

- `src/main.c` — código-fonte completo do simulador. Contém:
  - `converter_long`: converte e valida strings numéricas lidas do arquivo de entrada.
  - `parse_argumentos`: interpreta cada linha de tarefa do arquivo de entrada, validando o formato e a relação `BURST <= DEADLINE <= PERIODO`.
  - `main`: lê o arquivo de entrada, executa a simulação tick a tick (chegadas periódicas, detecção de deadlines perdidos, seleção de prioridade conforme o modo escolhido, execução e conclusão), e grava o resultado formatado no arquivo de saída.
- `Makefile` — compila o projeto e permite limpar os arquivos gerados.
- `README.md` — este arquivo.

## Sistema operacional utilizado

Desenvolvido e testado em Linux via WSL (Ubuntu), usando `gcc`.

## Como compilar

Na raiz do projeto, execute:

```
make
```

Isso gera o executável `scheduler` na raiz do projeto.

Para limpar o executável e os arquivos de saída gerados:

```
make clean
```

## Como executar

```
./scheduler rate <arquivo_de_entrada.txt>
./scheduler edf <arquivo_de_entrada.txt>
```

O primeiro argumento escolhe o algoritmo de escalonamento (`rate` ou `edf`); o
segundo é o caminho do arquivo de entrada.

### Formato do arquivo de entrada

```
[TEMPO TOTAL]
[NOME] [PERIODO] [DEADLINE] [BURST]
[NOME] [PERIODO] [DEADLINE] [BURST]
...
```

Exemplo (`voo.txt`):

```
100
ATT 20 12 8
NAV 50 30 15
```

### Saída

O programa grava o resultado no arquivo `rate_lhlc.out` ou `edf_lhlc.out`,
conforme o modo escolhido, sem imprimir nada na saída padrão durante a
execução normal. O arquivo contém:

- A sequência de execução, agrupada em blocos, com a duração e o motivo do
  encerramento de cada bloco (`F` = concluída, `H` = interrompida por outra
  tarefa, `L` = perdeu o deadline, `K` = ainda em execução ao final da
  simulação).
- A contagem de instâncias com deadline perdido (`LOST DEADLINES`).
- A contagem de instâncias concluídas com sucesso (`COMPLETE EXECUTION`).
- A contagem de instâncias que ainda estavam em execução quando a simulação
  terminou (`KILLED`).

## Como testar

1. Crie um arquivo de entrada seguindo o formato acima (ou use o exemplo do
   enunciado, `voo.txt`).
2. Compile com `make`.
3. Execute `./scheduler rate voo.txt` e `./scheduler edf voo.txt`.
4. Confira o conteúdo de `rate_lhlc.out` e `edf_lhlc.out`.

O programa também trata e rejeita entradas inválidas (número incorreto de
argumentos na linha de comando, modo diferente de `rate`/`edf`, arquivo
inexistente ou ilegível, arquivo malformado, e tarefas que violam
`BURST <= DEADLINE <= PERIODO`), retornando um código de saída diferente de
zero e uma mensagem de erro em stderr, sem gerar arquivo de saída.