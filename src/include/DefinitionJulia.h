#ifndef DEFINITIONJULIA_H
#define DEFINITIONJULIA_H

#include <stdbool.h>

// --- MACRO DE EXPORTAÇÃO (O Segredo do Windows) ---
#ifdef _WIN32
    #define AQUA_EXPORT __declspec(dllexport)
#else
    #define AQUA_EXPORT
#endif

// As ações possíveis que a Julia pode comandar
typedef enum {
    ACAO_ESPERAR = 0,
    ACAO_CIMA = 1,
    ACAO_BAIXO = 2,
    ACAO_FRENTE = 3,
    ACAO_TRAS = 4,
    ACAO_CAPTURAR = 5
} AcaoBot;

// O Vetor de Estado (A visão de mundo da IA)
typedef struct {
    float oxigenio;
    float distInimigo;
    float distBolha;
    float pontuacao;
} EstadoAquapura;


// --- API PARA O SEU MAIN.C ---

// Prepara e cacheia as funções da Julia
bool prepararModelosIA();

// Envia o frame atual para o modelo e retorna o comando (Substituiu o preverAcaoBot)
AcaoBot obterDecisaoDaIA(EstadoAquapura estadoAtual);

// Uma função de teste que envia dados para a Julia
float testarBidirecionalidade(float distanciaPercorrida, float energiaAtual);


// --- FUNÇÃO DO C (A Julia vai chamar esta) ---
AQUA_EXPORT void C_DispararEfeitoArcade(int tipoEfeito, float intensidade);

#endif // DEFINITIONJULIA_H