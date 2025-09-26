#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <functional>
#include "logging.hpp"

using namespace std;
using namespace Colors;

#define qtd_threads 10

Logging logger;
pthread_barrier_t barrier;

// Array de cores para sortear

void* thread_function(void* args) {
    int id = *((int*) args);

    cout << "Thread " << id+1 << " inicializada, esperando na barreira..." << endl;

    // Sincroniza todas as threads
    pthread_barrier_wait(&barrier);

    if (id == 0) {
        cout << "\nIniciando o teste da biblioteca Logging criada\n" << endl;
    }

    // Sorteia funcao
    int idx = (rand() % 5) + 1;

    // Usa logger com ticket lock em funcao aleatoria
    switch(idx){
        case 1: logger.log("Thread " + to_string(id+1) + " testando e finalizando..."); break;
        case 2: logger.error("Thread " + to_string(id+1) + " testando e finalizando..."); break;
        case 3: logger.info("Thread " + to_string(id+1) + " testando e finalizando..."); break;
        case 4: logger.success("Thread " + to_string(id+1) + " testando e finalizando..."); break;
        case 5: logger.warning("Thread " + to_string(id+1) + " testando e finalizando..."); break;
    }

    pthread_exit(nullptr);
}

int main() {
    srand(time(nullptr));

    pthread_t threads[qtd_threads];
    int ids[qtd_threads];

    cout << "===== TESTE DA CLASSE LOGGING =====" << endl;
    cout << "Quantidade de Threads: " << qtd_threads << endl;

    // Inicializa a barreira
    pthread_barrier_init(&barrier, nullptr, qtd_threads);

    cout << "\nCriando as Threads..." << endl;
    for (int i = 0; i < qtd_threads; i++) {
        ids[i] = i;
        pthread_create(&threads[i], nullptr, thread_function, &ids[i]);
    }

    // Espera todas terminarem
    for (int i = 0; i < qtd_threads; i++) {
        pthread_join(threads[i], nullptr);
    }

    // Destroi a barreira
    pthread_barrier_destroy(&barrier);

    cout << "\n===== FINALIZADO =====" << endl;
    return 0;
}
