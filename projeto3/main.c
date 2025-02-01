#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

typedef struct Log {
    char message[BUFFER_SIZE];
    struct Log *next;
} Log;

Log *log_head = NULL;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t start_time;
volatile sig_atomic_t running = 1;

// Libera a memória dos logs
void free_log() {
    pthread_mutex_lock(&log_mutex);
    Log *current = log_head;
    while (current) {
        Log *temp = current;
        current = current->next;
        free(temp);
    }
    log_head = NULL;
    pthread_mutex_unlock(&log_mutex);
}

// Adiciona log à lista
void add_log(const char *message) {
    pthread_mutex_lock(&log_mutex);
    Log *new_log = malloc(sizeof(Log));
    strncpy(new_log->message, message, BUFFER_SIZE);
    new_log->next = log_head;
    log_head = new_log;
    pthread_mutex_unlock(&log_mutex);
}

// Thread para exibir e limpar logs a cada 10 segundos
void *log_thread_func(void *arg) {
    while (running) {
        sleep(10);
        pthread_mutex_lock(&log_mutex);
        Log *current = log_head;
        Log *prev = NULL;
        while (current) {
            printf("%s", current->message);
            prev = current;
            current = current->next;
            free(prev);
        }
        log_head = NULL;
        pthread_mutex_unlock(&log_mutex);
    }
    return NULL;
}

// Processa comandos do cliente
void handle_command(int client_socket, const char *command) {
    char response[BUFFER_SIZE] = {0};
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char log_entry[BUFFER_SIZE];

    if (strncmp(command, "DATETIME", 8) == 0) {
        strftime(response, sizeof(response), "%Y-%m-%d %H:%M:%S\n", t);
    } else if (strncmp(command, "RNDNUMBER", 9) == 0) {
        snprintf(response, sizeof(response), "%d\n", rand() % 100 + 1);
    } else if (strncmp(command, "UPTIME", 6) == 0) {
        snprintf(response, sizeof(response), "Tempo de execução: %ld segundos\n", now - start_time);
    } else if (strncmp(command, "INFO", 4) == 0) {
        strncpy(response, "Servidor TCP v2.0\n", sizeof(response));
    } else if (strncmp(command, "BYE", 3) == 0) {
        strncpy(response, "Conexão encerrada\n", sizeof(response));
    } else {
        strncpy(response, "Comando inválido\n", sizeof(response));
    }

    send(client_socket, response, strlen(response), 0);

    // Formata log
    snprintf(log_entry, sizeof(log_entry), "[%04d-%02d-%02d %02d:%02d:%02d] Cliente %d: %s -> %s",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec,
             client_socket, command, response);
    add_log(log_entry);
}

// Thread para lidar com cliente
void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) break;
        buffer[bytes_read] = '\0';
        handle_command(client_socket, buffer);
        if (strstr(buffer, "BYE")) break;
    }

    close(client_socket);
    return NULL;
}

// Encerra o servidor via Ctrl+C
void handle_signal(int sig) {
    running = 0;
    printf("\nEncerrando servidor...\n");
    free_log();
    exit(0);
}

int main() {
    signal(SIGINT, handle_signal);
    srand(time(NULL));
    start_time = time(NULL);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t log_thread;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erro ao criar socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erro ao fazer bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Erro ao ouvir");
        close(server_socket);
        return 1;
    }

    pthread_create(&log_thread, NULL, log_thread_func, NULL);
    printf("Servidor TCP rodando na porta %d (Ctrl+C para sair)\n", PORT);

    while (running) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket == -1) {
            perror("Erro ao aceitar conexão");
            continue;
        }

        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_handler, client_sock_ptr);
        pthread_detach(client_thread);
    }

    close(server_socket);
    free_log();
    return 0;
}