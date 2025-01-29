#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_SIMULACAO 2  // Número máximo de execuções do simulador

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger o estoque
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para sincronizar saída
pthread_barrier_t barrier_1, barrier_2;             // Barreiras para sincronização
int estoque = 0;                               // Estoque global compartilhado

// Função para a área de inspeção dos robôs
void* inspecionar_item(void* arg) {
    long id = (long) arg;

    for (int i = 0; i < NUM_SIMULACAO; i++) {
        pthread_mutex_lock(&print_mutex);
        printf("Robô de inspeção %ld está pronto para inspecionar.\n", id);
        pthread_mutex_unlock(&print_mutex);

        // Aguarda todos os robôs e caminhões estarem prontos
        pthread_barrier_wait(&barrier_1);

        // Simula tempo de inspeção fixo (100ms)
        usleep(100000);

        // Seção crítica: inspeciona e estoca um item
        pthread_mutex_lock(&mutex);
        estoque++;
        pthread_mutex_lock(&print_mutex);
        printf("Robô de inspeção %ld inspecionou e estocou um item. Itens no estoque: %d\n", id, estoque);
        pthread_mutex_unlock(&print_mutex);
        pthread_mutex_unlock(&mutex);

        // Aguarda todos terminarem a inspeção antes de liberar transporte
        pthread_barrier_wait(&barrier_2);
    }

    return NULL;
}

// Função para a área de transporte dos caminhões
void* carregar_item(void* arg) {
    long id = (long) arg;

    for (int i = 0; i < NUM_SIMULACAO; i++) {
        pthread_mutex_lock(&print_mutex);
        printf("Caminhão %ld está pronto para carregar.\n", id);
        pthread_mutex_unlock(&print_mutex);

        // Aguarda todos os robôs e caminhões estarem prontos
        pthread_barrier_wait(&barrier_1);

        // Aguarda todos os robôs completarem a inspeção
        pthread_barrier_wait(&barrier_2);

        // Simula tempo de transporte fixo (100ms)
        usleep(100000);

        // Seção crítica: carrega e remove um item do estoque
        pthread_mutex_lock(&mutex);
        if (estoque > 0) {
            estoque--;
            pthread_mutex_lock(&print_mutex);
            printf("Caminhão %ld carregou um item. Itens restantes: %d\n", id, estoque);
            pthread_mutex_unlock(&print_mutex);
        } else {
            pthread_mutex_lock(&print_mutex);
            printf("Caminhão %ld tentou carregar, mas o estoque está vazio.\n", id);
            pthread_mutex_unlock(&print_mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    int num_robos, num_caminhoes;

    // Validação de entrada: número de robôs
    printf("Digite o número de robôs de inspeção: ");
    if (scanf("%d", &num_robos) != 1 || num_robos <= 0) {
        printf("Erro: Número de robôs inválido.\n");
        exit(EXIT_FAILURE);
    }

    // Validação de entrada: número de caminhões
    printf("Digite o número de caminhões de transporte: ");
    if (scanf("%d", &num_caminhoes) != 1 || num_caminhoes <= 0) {
        printf("Erro: Número de caminhões inválido.\n");
        exit(EXIT_FAILURE);
    }

    pthread_t robos[num_robos], caminhoes[num_caminhoes];

    // Inicializa barreiras
    pthread_barrier_init(&barrier_1, NULL, num_robos + num_caminhoes);
    pthread_barrier_init(&barrier_2, NULL, num_robos + num_caminhoes);

    // Cria threads de robôs com tratamento de erro
    for (long i = 0; i < num_robos; i++) {
        if (pthread_create(&robos[i], NULL, inspecionar_item, (void*)(i + 1)) != 0) {
            perror("Erro ao criar thread de robô");
            exit(EXIT_FAILURE);
        }
    }

    // Cria threads de caminhões com tratamento de erro
    for (long i = 0; i < num_caminhoes; i++) {
        if (pthread_create(&caminhoes[i], NULL, carregar_item, (void*)(i + 1)) != 0) {
            perror("Erro ao criar thread de caminhão");
            exit(EXIT_FAILURE);
        }
    }

    // Aguarda término das threads
    for (int i = 0; i < num_robos; i++) {
        pthread_join(robos[i], NULL);
    }
    for (int i = 0; i < num_caminhoes; i++) {
        pthread_join(caminhoes[i], NULL);
    }

    // Destrói recursos de sincronização
    pthread_barrier_destroy(&barrier_1);
    pthread_barrier_destroy(&barrier_2);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&print_mutex);

    printf("Operações concluídas. Estoque final: %d\n", estoque);
    return 0;
}