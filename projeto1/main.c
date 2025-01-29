#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h> 

#define NUM_LETRAS 26
#define ASCII_A 65
#define TAMANHO_SENHA 4
#define NUM_THREADS 10

typedef struct {
    char** senhas;
    int id;
    int num_senhas;
} ThreadArgs;

char* encrypt(const char* str) {
    char* str_result = (char*) malloc(sizeof(char) * (TAMANHO_SENHA + 1));

    if (!str_result) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < TAMANHO_SENHA; i++) {
        char c = str[i];
        char chave = str[i];
        if (c >= 'A' && c <= 'Z') {
            int str_idx = c - ASCII_A;
            int chave_idx = chave - ASCII_A;
            str_result[i] = ((str_idx + chave_idx) % NUM_LETRAS) + ASCII_A;
        } else {
            perror("Erro: String contém caracteres inválidos.");
            free(str_result);
            exit(EXIT_FAILURE);
        }
    }
    str_result[TAMANHO_SENHA] = '\0';
    return str_result;
}

char** ler_arquivo(const char* nome_arquivo, int* num_senhas) {
    FILE *fp;
    char linha[10];
    char **linhas = NULL;
    int num_linhas = 0;

    fp = fopen(nome_arquivo, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    while (fgets(linha, sizeof(linha), fp) != NULL) {
        num_linhas++;
    }
    rewind(fp);

    linhas = (char**)malloc(num_linhas * sizeof(char*));
    if (linhas == NULL) {
        perror("Erro ao alocar memória");
        fclose(fp);
        return NULL;
    }

    int i = 0;
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        linha[strcspn(linha, "\n")] = '\0';
        linhas[i] = (char*)malloc(strlen(linha) + 1);
        if (linhas[i] == NULL) {
            perror("Erro ao alocar memória");
            for (int j = 0; j < i; j++) {
                free(linhas[j]);
            }
            free(linhas);
            fclose(fp);
            return NULL;
        }
        strcpy(linhas[i], linha);
        i++;
    }

    fclose(fp);
    *num_senhas = num_linhas;
    return linhas;
}

char** decrypt(const char* str, int* count) {
    char tentativa[TAMANHO_SENHA + 1];
    char** resultados = NULL;
    int capacidade = 10;
    *count = 0;

    resultados = (char**)malloc(capacidade * sizeof(char*));
    if (!resultados) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_LETRAS; i++) {
        for (int j = 0; j < NUM_LETRAS; j++) {
            for (int k = 0; k < NUM_LETRAS; k++) {
                for (int l = 0; l < NUM_LETRAS; l++) {
                    tentativa[0] = ASCII_A + i;
                    tentativa[1] = ASCII_A + j;
                    tentativa[2] = ASCII_A + k;
                    tentativa[3] = ASCII_A + l;
                    tentativa[4] = '\0';

                    char* tentativa_ec = encrypt(tentativa);

                    if (strcmp(tentativa_ec, str) == 0) {
                        if (*count >= capacidade) {
                            capacidade *= 2;
                            resultados = (char**)realloc(resultados, capacidade * sizeof(char*));
                            if (!resultados) {
                                perror("Erro ao realocar memória");
                                exit(EXIT_FAILURE);
                            }
                        }
                        resultados[*count] = strdup(tentativa);
                        (*count)++;
                    }

                    free(tentativa_ec);
                }
            }
        }
    }

    return resultados;
}

void* thread_decrypt(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    char output_filename[40];
    sprintf(output_filename, "resultados/dec_passwd_%d.txt", t_args->id);

    FILE *fp = fopen(output_filename, "w");
    if (fp == NULL) {
        perror("Erro ao criar arquivo de saída");
        return NULL;
    }

    printf("Thread #%d: Quebrando senhas de passwd_%d.txt\n", t_args->id, t_args->id);

    for (int i = 0; i < t_args->num_senhas; i++) {
        int count;
        char** resultados = decrypt(t_args->senhas[i], &count);

        fprintf(fp, "Senha criptografada: %s -> Senhas quebradas: ", t_args->senhas[i]);
        for (int j = 0; j < count; j++) {
            fprintf(fp, "%s", resultados[j]);
            if (j < count - 1) {
                fprintf(fp, ";");
            }
            free(resultados[j]);
        }
        fprintf(fp, "\n");
        free(resultados);
    }

    fclose(fp);
    printf("Thread #%d: Senhas quebradas salvas em %s\n", t_args->id, output_filename);

    return NULL;
}

void process_decrypt(int id, char** senhas, int num_senhas) {
    char output_filename[40];
    sprintf(output_filename, "resultados/dec_passwd_%d.txt", id);

    FILE *fp = fopen(output_filename, "w");
    if (fp == NULL) {
        perror("Erro ao criar arquivo de saída");
        exit(EXIT_FAILURE);
    }

    printf("Processo PID %d: Quebrando senhas de passwd_%d.txt\n", getpid(), id);

    for (int i = 0; i < num_senhas; i++) {
        int count;
        char** resultados = decrypt(senhas[i], &count);

        fprintf(fp, "Senha criptografada: %s -> Senhas quebradas: ", senhas[i]);
        for (int j = 0; j < count; j++) {
            fprintf(fp, "%s", resultados[j]);
            if (j < count - 1) {
                fprintf(fp, ";");
            }
            free(resultados[j]);
        }
        fprintf(fp, "\n");
        free(resultados);
    }

    fclose(fp);
    printf("Processo PID %d: Senhas quebradas salvas em %s\n", getpid(), output_filename);
}

int main() {
    char **senhas[NUM_THREADS];
    int num_senhas[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    pid_t pids[NUM_THREADS];
    int escolha;
    clock_t start, end;
    double cpu_time_used;

    // Cria as pastas se não existirem
    mkdir("senhas", 0777);
    mkdir("resultados", 0777);

    // Verifica os arquivos de entrada
    for (int i = 0; i < NUM_THREADS; i++) {
        char filename[30];
        sprintf(filename, "senhas/passwd_%d.txt", i);
        senhas[i] = ler_arquivo(filename, &num_senhas[i]);

        if (senhas[i] == NULL) {
            fprintf(stderr, "Erro: Arquivo '%s' não encontrado.\n", filename);
            exit(EXIT_FAILURE);
        }
    }

    printf("Escolha o método de execução:\n1. Processos\n2. Threads\n");
    int resultado_scan = scanf("%d", &escolha);
    if (resultado_scan != 1) {
        printf("Erro: Entrada inválida. Use apenas números (1 ou 2).\n");
        exit(EXIT_FAILURE);
    }

    start = clock();

    if (escolha == 2) {
        printf("Criando 10 threads para processar arquivos...\n");
        for (int i = 0; i < NUM_THREADS; i++) {
            ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
            args->senhas = senhas[i];
            args->id = i;
            args->num_senhas = num_senhas[i];
            if (pthread_create(&threads[i], NULL, thread_decrypt, (void*)args) != 0) {
                perror("Erro ao criar thread");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
    } else if (escolha == 1) {
        printf("Gerando 10 processos para processar arquivos...\n");
        for (int i = 0; i < NUM_THREADS; i++) {
            pids[i] = fork();
            if (pids[i] == -1) {
                perror("Erro ao criar processo");
                exit(EXIT_FAILURE);
            } else if (pids[i] == 0) {
                process_decrypt(i, senhas[i], num_senhas[i]);
                exit(0);
            }
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            wait(NULL);
        }
    } else {
        printf("Opção inválida.\n");
        exit(EXIT_FAILURE);
    }

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Tempo total de execução: %f segundos\n", cpu_time_used);

    return 0;
}