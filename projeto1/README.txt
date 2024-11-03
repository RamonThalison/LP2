# Projeto de Decodificação de Senhas

Este programa decodifica senhas encriptadas usando múltiplas threads ou processos. As senhas possíveis são salvas em arquivos separados, com todas as possibilidades separadas por ponto e vírgula.

## Instruções de Compilação

Para compilar o programa, use o seguinte comando:
gcc funcional.c -o programa

## Instruções de Execução

Para executar o programa, use o comando:
./programa

Você será solicitado a escolher entre threads ou processos:
- Digite 1 para usar processos.
- Digite 2 para usar threads.

## Estrutura dos Arquivos de Entrada e Saída

O programa espera arquivos de entrada com senhas encriptadas, nomeados como passwd_0.txt, passwd_1.txt, etc.
As senhas decodificadas serão salvas em arquivos dec_passwd_0.txt, dec_passwd_1.txt, etc., com todas as possíveis senhas separadas por ponto e vírgula.

## Observações

Certifique-se de que todos os arquivos passwd_*.txt estejam na mesma pasta que o programa durante a execução.

