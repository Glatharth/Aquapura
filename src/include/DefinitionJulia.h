#ifndef DEFINITIONJULIA_H
#define DEFINITIONJULIA_H

#include <stdbool.h>

#ifdef _WIN32
    #define AQUA_EXPORT __declspec(dllexport)
#else
    #define AQUA_EXPORT
#endif

typedef enum {
    ACAO_ESPERAR = 0, ACAO_CIMA = 1, ACAO_BAIXO = 2,
    ACAO_FRENTE = 3, ACAO_TRAS = 4, ACAO_CAPTURAR = 5
} AcaoBot;

// Prepara e cacheia as funções da Julia
bool prepararModelosIA();

// Envia a população inteira para a Julia e preenche o vetor de ações retornado
void obterDecisoesDaPopulacao(double *vetorEstados, int numJogadores, int *vetorAcoes);

// Avisa a Julia que a geração acabou (todos morreram) e passa as pontuações
void notificarFimDeGeracao(double *vetorPontuacoes, int numJogadores);

// --- FUNÇÃO DO C (A Julia vai chamar esta para reiniciar) ---
AQUA_EXPORT void C_ReiniciarMundo(int quantidadeDeJogadores);

#endif // DEFINITIONJULIA_H