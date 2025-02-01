# Servidor TCP Multithread

Servidor que processa comandos de clientes e registra logs em tempo real.

## Comandos Suportados:
- `DATETIME`: Retorna a data e hora atual.
- `RNDNUMBER`: Gera um número aleatório entre 1 e 100.
- `UPTIME`: Mostra o tempo de execução do servidor.
- `INFO`: Exibe a versão do servidor.
- `BYE`: Encerra a conexão com o cliente.

## Como Usar:
1. **Compilar:**
   ```bash
   make