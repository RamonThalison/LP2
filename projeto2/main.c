#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // Para usar usleep

#define NUM_SIMULACAO 2  // Número máximo de execuções do simulador

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex global para proteger o estoque
pthread_barrier_t barrier_1, barrier_2;             // Duas barreiras para sincronização
int estoque = 0;                               // Estoque global compartilhado

// Função para a área de inspeção dos robôs
void* inspecionar_item(void* arg) {
    long id = (long) arg;

    for (int i = 0; i < NUM_SIMULACAO; i++) {
        printf("Robô de inspeção %ld está pronto para inspecionar.\n", id);

        // Aguarda todos os robôs e caminhões estarem prontos
        pthread_barrier_wait(&barrier_1);

        // Simula tempo de inspeção com usleep
        usleep(100 * id);

        // Seção crítica: inspeciona e estoca um item
        pthread_mutex_lock(&mutex);
        estoque++;
        printf("Robô de inspeção %ld inspecionou e estocou um item. Itens no estoque: %d\n", id, estoque);
        pthread_mutex_unlock(&mutex);

        // Aguarda todos os robôs terminarem a inspeção antes de liberar o transporte
        pthread_barrier_wait(&barrier_2);
    }

    return NULL;
}

// Função para a área de transporte dos caminhões
void* carregar_item(void* arg) {
    long id = (long) arg;

    for (int i = 0; i < NUM_SIMULACAO; i++) {
        printf("Caminhão %ld está pronto para carregar.\n", id);

        // Aguarda todos os robôs e caminhões estarem prontos
        pthread_barrier_wait(&barrier_1);

        // Aguarda todos os robôs completarem a inspeção antes de carregar
        pthread_barrier_wait(&barrier_2);

        // Simula tempo de transporte com usleep
        usleep(100 * id);

        // Seção crítica: carrega e remove um item do estoque
        pthread_mutex_lock(&mutex);
        if (estoque > 0) {
            estoque--;
            printf("Caminhão %ld carregou um item. Itens restantes no estoque: %d\n", id, estoque);
        } else {
            printf("Caminhão %ld tentou carregar, mas o estoque está vazio.\n", id);
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    int num_robos, num_caminhoes;

    // Solicita o número de robôs e caminhões ao usuário
    printf("Digite o número de robôs de inspeção: ");
    scanf("%d", &num_robos);
    printf("Digite o número de caminhões de transporte: ");
    scanf("%d", &num_caminhoes);

    pthread_t robos[num_robos], caminhoes[num_caminhoes];

    // Inicializa as barreiras
    pthread_barrier_init(&barrier_1, NULL, num_robos + num_caminhoes);
    pthread_barrier_init(&barrier_2, NULL, num_robos + num_caminhoes);

    // Cria threads de robôs de inspeção
    for (long i = 0; i < num_robos; i++) {
        pthread_create(&robos[i], NULL, inspecionar_item, (void*) (i + 1));
    }

    // Cria threads de caminhões de transporte
    for (long i = 0; i < num_caminhoes; i++) {
        pthread_create(&caminhoes[i], NULL, carregar_item, (void*) (i + 1));
    }

    // Aguarda o término das threads dos robôs
    for (int i = 0; i < num_robos; i++) {
        pthread_join(robos[i], NULL);
    }

    // Aguarda o término das threads dos caminhões
    for (int i = 0; i < num_caminhoes; i++) {
        pthread_join(caminhoes[i], NULL);
    }

    // Destroi as barreiras
    pthread_barrier_destroy(&barrier_1);
    pthread_barrier_destroy(&barrier_2);

    // Destroi o mutex
    pthread_mutex_destroy(&mutex);

    printf("Todos os robôs de inspeção e caminhões completaram suas operações.\n");

    return 0;
}
