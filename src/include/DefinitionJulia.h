#ifndef DEFINITIONJULIA_H
#define DEFINITIONJULIA_H

#include <stdbool.h>

#ifdef _WIN32
    #define AQUA_EXPORT __declspec(dllexport)
#else
    #define AQUA_EXPORT
#endif

// Prepara e cacheia as funções da Julia
bool prepararModelosIA();

// Envia a população inteira para a Julia e preenche o vetor de ações retornado
void obterDecisoesDaPopulacao(double *vetorEstados, int numJogadores, int *vetorAcoes);

// Avisa a Julia que a geração acabou (todos morreram) e passa as pontuações
void notificarFimDeGeracao(double *vetorPontuacoes, int numJogadores);

// --- FUNÇÃO DO C (A Julia vai chamar esta para reiniciar) ---
AQUA_EXPORT void C_ReiniciarMundo(int quantidadeDeJogadores);
AQUA_EXPORT void C_ConfigurarDificuldade(float spawnInterval, int npcSpeed);
AQUA_EXPORT void C_ConfigurarSpawn(int probAnimal, int probGarbage);

#endif // DEFINITIONJULIA_H