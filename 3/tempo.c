//                              Sistemas Distribuídos
//                              Autor: Leonardo Luan
                              
//                              Data da última modificação: 01/11/2021

#include<stdio.h>
#include<stdlib.h>
#include "smpl.h"

// ------------------ Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3

// ------------------ Descritor do NODO SMPL
typedef struct {
    int id;             // identificador da facility SMPL
    int* state
} tnodo;

tnodo *nodo;

void updateState(tnodo nodo, int token, int status) {
    int lastStatus = nodo.state[token] % 2;

    if (lastStatus != status) {
        nodo.state[token]++;
    }
}

void printState(tnodo nodo, int token, int n) {
    printf("%d STATE = [ ", token);
    for (int x = 0; x < n; x++) printf("%d ", nodo.state[x]);
    printf("]\n\n");
}

int main(int argc, char* argv[]) {

    static int n;           // número total de processos
    static int token;       // processo que "está executando"
    static int event;       //
    static int r, i, j;        //

    static char fa_name[5]; // nome da facility

    if (argc != 2) {
        puts("Uso correto: tempo <num-nodos>");
        exit(1);
    }

    smpl(0, "Um exemplo de simulação");
    reset();
    stream(1);

    // init
    n = atoi(argv[1]);
    nodo = (tnodo*) malloc(sizeof(tnodo)*n);

    for(i=0; i<n; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);

        nodo[i].state = (int*) malloc(sizeof(int)*n);
        for (j = 0; j < n; j++) {
            nodo[i].state[j] = -1;
        }
    }

    // escalonar os eventos
    for (i=0; i<n; i++) {
        schedule(TEST, 30.0, i);                // escalonando testes para todos os nodos executarem em t=30
    }

    schedule(FAULT, 31.0, 1);                   // o processo 1 falha em t=31
    schedule(REPAIR, 121.0, 1);                  // o processo 1 recupera em t=121

    while(time() < 150.0) {
        
        cause(&event, &token);
        
        switch (event) {
        case TEST:
            if (status(nodo[token].id) != 0) break;

            int next = (token + 1)%n; // prox NODO

            while (status(nodo[next].id) == 1) {
                updateState(nodo[token], next, 1);
                printf("O NODO %d testou o NODO %d no tempo %4.1f como FALHO\n", token, next, time());
                printState(nodo[token], token, n);
                next = (next + 1)%n;
            }

            if (next != token) { // para nao se auto testar
                updateState(nodo[token], next, 0);
                printf("O NODO %d testou o NODO %d no tempo %4.1f como CORRETO\n", token, next, time());
                printState(nodo[token], token, n);
            }

            schedule(TEST, 30.0, token);
            break;
        case FAULT:
            r = request(nodo[token].id, token, 0); 
            if (r != 0) {
                puts("Não foi possível falhar o NODO...");
                break;
            }
            printf("O NODO %d falhou no tempo %4.1f\n\n", token, time());
            break;
        case REPAIR:
            release(nodo[token].id, token);
            printf("O NODO %d recuperou no tempo %4.1f\n\n", token, time());
            schedule(TEST, 30.0, token);
            break;
        }       // switch
    }           // while
}               // main
