#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


typedef struct tarefa {
    char nome[50];
    long PERIODO;
    long BURST;
    long DEADLINE;
    long deadline_absoluto;
    long contador_perdidas;
    long contador_mortas;
    long contador_concluidas;
    long tempo_restante;

} tarefa;


int converter_long(const char *str, long *saida) {

    errno = 0;

    char *endptr;

    long valor = strtol(str, &endptr, 10);

    if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r' && *endptr != ' ') {
        return 0;
    }

    if (errno == ERANGE) {
        return 0;
    }

    if (valor <= 0) {
        return 0;
    }

    *saida = valor;

    return 1;
}

int parse_argumentos(char *args, tarefa *novaTarefa) {

    int contador = 0;

    char *token = strtok(args, " \t\n\r");

    if (token == NULL) {
        return 0;
    }

    strcpy(novaTarefa->nome, token);

    token = strtok(NULL, " \t\n\r");

    while (token != NULL) {

        if (contador >= 3) {
            fprintf(stderr, "Error: Numero de argumentos invalido. Esperado 4 argumentos.\n");
            return 0;
        }

        if (contador == 0) {
            if (!converter_long(token, &novaTarefa->PERIODO)) {
                return 0;
            }
        } else if (contador == 1) {
            if (!converter_long(token, &novaTarefa->DEADLINE)) {
                return 0;
            }
        } else if (contador == 2) {
            if (!converter_long(token, &novaTarefa->BURST)) {
                return 0;
            }
        }

        contador++;

        token = strtok(NULL, " \t\n\r");
    }

    if (contador != 3) {
        fprintf(stderr, "Error: Numero de argumentos invalido. Esperado 4 argumentos.\n");
        return 0;
    }

    if (contador == 3) {

        if(!(novaTarefa->BURST <= novaTarefa->DEADLINE && novaTarefa->DEADLINE <= novaTarefa->PERIODO)) {
            fprintf(stderr, "Error: Argumentos invalidos. Esperado BURST <= DEADLINE <= PERIODO.\n");
            return 0;
        }
    }

    return 1;
}


int main(int argc, char *argv[]) {
    
    tarefa listaTarefas[100];
    int contadorTarefas = 0;

    if (argc != 3) {
        fprintf(stderr, "Error: <./scheduler> <modo> <arquivo.txt>\n");
        exit(1);
    }
    
    if(strcmp(argv[1], "rate") == 0) {

        FILE *arquivo = fopen(argv[2], "r");
        if (arquivo == NULL) {
            fprintf(stderr, "Error: Nao foi possivel abrir o arquivo '%s'.\n", argv[2]);
            exit(1);
        }
        
        char linha[256];
        
        if(fgets(linha, sizeof(linha), arquivo) == NULL) {
            fprintf(stderr, "Error: Arquivo vazio ou falha ao ler o tempo total da simulacao.");
            fclose(arquivo);
            exit(1);

        }

        long tempoTotalSimulacao;
        
        if (!converter_long(linha, &tempoTotalSimulacao)) {
            fprintf(stderr, "Error: Falha ao processar o tempo total de simulacao: %s\n", linha);
            fclose(arquivo);
            exit(1);
        }

        while (fgets(linha, sizeof(linha), arquivo)) {
            tarefa novaTarefa;

            if (!parse_argumentos(linha, &novaTarefa)) {
                fprintf(stderr, "Error: Falha ao processar a linha: %s\n", linha);
                fclose(arquivo);
                exit(1);
            } else {
                if (contadorTarefas >= 100) {
                    fprintf(stderr, "Error: Lista de tarefas cheia.");
                    fclose(arquivo);
                    exit(1);
                }

                strcpy(listaTarefas[contadorTarefas].nome, novaTarefa.nome);
                listaTarefas[contadorTarefas].PERIODO = novaTarefa.PERIODO;
                listaTarefas[contadorTarefas].BURST = novaTarefa.BURST;
                listaTarefas[contadorTarefas].DEADLINE = novaTarefa.DEADLINE;
                listaTarefas[contadorTarefas].deadline_absoluto = 0;
                listaTarefas[contadorTarefas].contador_perdidas = 0;
                listaTarefas[contadorTarefas].contador_mortas = 0;
                listaTarefas[contadorTarefas].contador_concluidas = 0;
                listaTarefas[contadorTarefas].tempo_restante = 0;
                contadorTarefas++;
                
            }
            
        }
        
        fclose(arquivo);

        FILE *saida = fopen("rate_lhlc.out", "w");
        if (saida == NULL) {
            fprintf(stderr, "Error: Nao foi possivel abrir arquivo de saida\n");
            exit(1);
        }
        fprintf(saida, "EXECUTION BY RATE\n");

        long instante_atual;
        char registroEvento[tempoTotalSimulacao];
        long registroExecucao[tempoTotalSimulacao];

        for(int i = 0; i < tempoTotalSimulacao; i++) {
            registroEvento[i] = ' ';
        }

        for(instante_atual = 0; instante_atual < tempoTotalSimulacao; instante_atual++) {
            for(long tarefa_atual = 0; tarefa_atual < contadorTarefas; tarefa_atual++) {
                if(instante_atual % listaTarefas[tarefa_atual].PERIODO == 0) {
                    listaTarefas[tarefa_atual].tempo_restante = listaTarefas[tarefa_atual].BURST;
                    listaTarefas[tarefa_atual].deadline_absoluto = instante_atual + listaTarefas[tarefa_atual].DEADLINE;
                }
    
                if(instante_atual == listaTarefas[tarefa_atual].deadline_absoluto && listaTarefas[tarefa_atual].tempo_restante > 0) {
                    listaTarefas[tarefa_atual].contador_perdidas++;
                    listaTarefas[tarefa_atual].tempo_restante = 0;
                    registroEvento[instante_atual] = 'L';
                }

            }

            long tarefa_escolhida = 0;
            for(long tarefa_atual = 0; tarefa_atual <= contadorTarefas - 1; tarefa_atual++) {

                if(listaTarefas[tarefa_atual].tempo_restante > 0 && (listaTarefas[tarefa_escolhida].tempo_restante == 0 || listaTarefas[tarefa_atual].PERIODO < listaTarefas[tarefa_escolhida].PERIODO)) {
                    tarefa_escolhida = tarefa_atual;
                }
                registroExecucao[instante_atual] = tarefa_escolhida;
            }

            if(listaTarefas[tarefa_escolhida].tempo_restante > 0) {
                listaTarefas[tarefa_escolhida].tempo_restante--;
                registroExecucao[instante_atual] = tarefa_escolhida;
                
                if(listaTarefas[tarefa_escolhida].tempo_restante == 0) {
                    listaTarefas[tarefa_escolhida].contador_concluidas++;
                    registroExecucao[instante_atual] = tarefa_escolhida;
                    registroEvento[instante_atual] = 'F';
                }
                if(instante_atual == tempoTotalSimulacao - 1) {
                    if(listaTarefas[tarefa_escolhida].tempo_restante > 0) {
                        registroExecucao[instante_atual] = tarefa_escolhida;
                        registroEvento[instante_atual] = 'K';
                        
                    }
                }
            } else {
                registroExecucao[instante_atual] = -1;
            }
            
        }

        for(instante_atual = 0; instante_atual < contadorTarefas; instante_atual++) {
            if(listaTarefas[instante_atual].tempo_restante > 0) {
                listaTarefas[instante_atual].contador_mortas++;
            }
        }

        long bloco_atual = registroExecucao[0];
        long duracao_tarefa = 1;

        for(instante_atual = 1; instante_atual < tempoTotalSimulacao; instante_atual++) {
            if(registroExecucao[instante_atual] == bloco_atual) {
                duracao_tarefa++;
            } else {
                if(bloco_atual == -1) {
                    fprintf(saida,"idle for %ld units\n", duracao_tarefa);
                } else {
                    char letra = registroEvento[instante_atual-1];
                    if(letra == ' ') {
                        letra = registroEvento[instante_atual];
                    }
                    if(letra == ' ') {
                        letra = 'H';
                    }
                    fprintf(saida, "[%s] for %ld units - %c\n", listaTarefas[bloco_atual].nome, duracao_tarefa, letra);
                }
                bloco_atual = registroExecucao[instante_atual];
                duracao_tarefa = 1;
            }
        }
        
        if(bloco_atual == -1) {
            fprintf(saida, "idle for %ld units\n", duracao_tarefa);
        } else {
            char letra = registroEvento[tempoTotalSimulacao- 1];
            if(letra == ' ') {
                letra = 'H';
            }
            fprintf(saida, "[%s] for %ld units - %c\n", listaTarefas[bloco_atual].nome, duracao_tarefa, letra);
        }

        fprintf(saida, "\n");
        fprintf(saida, "\nLOST DEADLINES\n");

        for(int i = 0; i < contadorTarefas; i++) {
            fprintf(saida, "[%s] %ld\n", listaTarefas[i].nome, listaTarefas[i].contador_perdidas);
        }

        fprintf(saida, "\nCOMPLETE EXECUTION\n");

        for(int i = 0; i < contadorTarefas; i++) {
            fprintf(saida, "[%s] %ld\n", listaTarefas[i].nome, listaTarefas[i].contador_concluidas);
        }

        fprintf(saida, "\nKILLED\n");

        for(int i = 0; i < contadorTarefas; i++) {
            fprintf(saida, "[%s] %ld\n", listaTarefas[i].nome, listaTarefas[i].contador_mortas);
        }

        fclose(saida);

    } else if(strcmp(argv[1], "edf") == 0) {

        FILE *arquivo = fopen(argv[2], "r");
        if (arquivo == NULL) {
            fprintf(stderr, "Error: Nao foi possivel abrir o arquivo '%s'.\n", argv[2]);
            exit(1);
        }
        
        char linha[256];
        
        if(fgets(linha, sizeof(linha), arquivo) == NULL) {
            fprintf(stderr, "Error: Arquivo vazio ou falha ao ler o tempo total da simulacao.");
            fclose(arquivo);
            exit(1);

        }

        long tempoTotalSimulacao;
        
        if (!converter_long(linha, &tempoTotalSimulacao)) {
            fprintf(stderr, "Error: Falha ao processar o tempo total de simulacao: %s\n", linha);
            fclose(arquivo);
            exit(1);
        }

        while (fgets(linha, sizeof(linha), arquivo)) {
            tarefa novaTarefa;

            if (!parse_argumentos(linha, &novaTarefa)) {
                fprintf(stderr, "Error: Falha ao processar a linha: %s\n", linha);
                fclose(arquivo);
                exit(1);
            } else {
                if (contadorTarefas >= 100) {
                    fprintf(stderr, "Error: Lista de tarefas cheia.");
                    fclose(arquivo);
                    exit(1);
                }

                strcpy(listaTarefas[contadorTarefas].nome, novaTarefa.nome);
                listaTarefas[contadorTarefas].PERIODO = novaTarefa.PERIODO;
                listaTarefas[contadorTarefas].BURST = novaTarefa.BURST;
                listaTarefas[contadorTarefas].DEADLINE = novaTarefa.DEADLINE;
                listaTarefas[contadorTarefas].deadline_absoluto = 0;
                listaTarefas[contadorTarefas].contador_perdidas = 0;
                listaTarefas[contadorTarefas].contador_mortas = 0;
                listaTarefas[contadorTarefas].contador_concluidas = 0;
                listaTarefas[contadorTarefas].tempo_restante = 0;
                contadorTarefas++;
                
            }
            
        }
        
        fclose(arquivo);

        FILE *saida = fopen("edf_lhlc.out", "w");
        if (saida == NULL) {
            fprintf(stderr, "Error: Nao foi possivel abrir arquivo de saida\n");
            exit(1);
        }
        fprintf(saida, "EXECUTION BY EDF\n");

        long instante_atual;
        char registroEvento[tempoTotalSimulacao];
        long registroExecucao[tempoTotalSimulacao];

        for(int i = 0; i < tempoTotalSimulacao; i++) {
            registroEvento[i] = ' ';
        }

        for(instante_atual = 0; instante_atual < tempoTotalSimulacao; instante_atual++) {
            for(long tarefa_atual = 0; tarefa_atual < contadorTarefas; tarefa_atual++) {
                if(instante_atual % listaTarefas[tarefa_atual].PERIODO == 0) {
                    listaTarefas[tarefa_atual].tempo_restante = listaTarefas[tarefa_atual].BURST;
                    listaTarefas[tarefa_atual].deadline_absoluto = instante_atual + listaTarefas[tarefa_atual].DEADLINE;
                }
    
                if(instante_atual == listaTarefas[tarefa_atual].deadline_absoluto && listaTarefas[tarefa_atual].tempo_restante > 0) {
                    listaTarefas[tarefa_atual].contador_perdidas++;
                    listaTarefas[tarefa_atual].tempo_restante = 0;
                    registroEvento[instante_atual] = 'L';
                }

            }

            long tarefa_escolhida = 0;
            for(long tarefa_atual = 0; tarefa_atual <= contadorTarefas - 1; tarefa_atual++) {

                if(listaTarefas[tarefa_atual].tempo_restante > 0 && (listaTarefas[tarefa_escolhida].tempo_restante == 0 || listaTarefas[tarefa_atual].deadline_absoluto < listaTarefas[tarefa_escolhida].deadline_absoluto)) {
                    tarefa_escolhida = tarefa_atual;
                }
                registroExecucao[instante_atual] = tarefa_escolhida;
            }

            if(listaTarefas[tarefa_escolhida].tempo_restante > 0) {
                listaTarefas[tarefa_escolhida].tempo_restante--;
                registroExecucao[instante_atual] = tarefa_escolhida;
                
                if(listaTarefas[tarefa_escolhida].tempo_restante == 0) {
                    listaTarefas[tarefa_escolhida].contador_concluidas++;
                    registroExecucao[instante_atual] = tarefa_escolhida;
                    registroEvento[instante_atual] = 'F';
                }
                if(instante_atual == tempoTotalSimulacao - 1) {
                    if(listaTarefas[tarefa_escolhida].tempo_restante > 0) {
                        registroExecucao[instante_atual] = tarefa_escolhida;
                        registroEvento[instante_atual] = 'K';
                        
                    }
                }
            } else {
                registroExecucao[instante_atual] = -1;
            }
            
        }

        for(instante_atual = 0; instante_atual < contadorTarefas; instante_atual++) {
            if(listaTarefas[instante_atual].tempo_restante > 0) {
                listaTarefas[instante_atual].contador_mortas++;
            }
        }

        long bloco_atual = registroExecucao[0];
        long duracao_tarefa = 1;

        for(instante_atual = 1; instante_atual < tempoTotalSimulacao; instante_atual++) {
            if(registroExecucao[instante_atual] == bloco_atual) {
                duracao_tarefa++;
            } else {
                if(bloco_atual == -1) {
                    fprintf(saida,"idle for %ld units\n", duracao_tarefa);
                } else {
                    char letra = registroEvento[instante_atual-1];
                    if(letra == ' ') {
                        letra = registroEvento[instante_atual];
                    }
                    if(letra == ' ') {
                        letra = 'H';
                    }
                    fprintf(saida, "[%s] for %ld units - %c\n", listaTarefas[bloco_atual].nome, duracao_tarefa, letra);
                }
                bloco_atual = registroExecucao[instante_atual];
                duracao_tarefa = 1;
            }
        }
        
        if(bloco_atual == -1) {
            fprintf(saida, "idle for %ld units\n", duracao_tarefa);
        } else {
            char letra = registroEvento[tempoTotalSimulacao- 1];
            if(letra == ' ') {
                letra = 'H';
            }
            fprintf(saida, "[%s] for %ld units - %c\n", listaTarefas[bloco_atual].nome, duracao_tarefa, letra);
        }

        fprintf(saida, "\nLOST DEADLINES\n");

        for(int i = 0; i < contadorTarefas; i++) {
            fprintf(saida, "[%s] %ld\n", listaTarefas[i].nome, listaTarefas[i].contador_perdidas);
        }

        fprintf(saida, "\nCOMPLETE EXECUTION\n");

        for(int i = 0; i < contadorTarefas; i++) {
            fprintf(saida, "[%s] %ld\n", listaTarefas[i].nome, listaTarefas[i].contador_concluidas);
        }

        fprintf(saida, "\nKILLED\n");

        for(int i = 0; i < contadorTarefas; i++) {
            fprintf(saida, "[%s] %ld\n", listaTarefas[i].nome, listaTarefas[i].contador_mortas);
        }

        fclose(saida);
    
    } else {
        fprintf(stderr, "Error: Modo invalido. Use 'rate' ou 'edf'.\n");
        exit(1);
    }

    return 0;
}