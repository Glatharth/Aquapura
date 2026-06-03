#ifndef DEFINITIONJULIA_H
#define DEFINITIONJULIA_H

#include <stdbool.h>

// --- ESTRUTURAS DO JOGO EM C PURO ---
// O resto do projeto usará apenas isso, sem saber da existência da Julia

typedef enum {
    ACAO_NENHUMA = 0,
    ACAO_PULAR = 1,
    ACAO_ABAIXAR = 2
} AcaoBot;

typedef struct {
    float posicaoX;
    float posicaoY;
    float distanciaProximoObstaculo;
} EstadoDoJogo;

// --- API DE COMUNICAÇÃO ---

// Busca as funções da Julia na memória e as deixa prontas para uso rápido
bool prepararModelosIA();

// Envia o frame atual para o modelo e retorna o comando
AcaoBot preverAcaoBot(EstadoDoJogo estadoAtual);

#endif // DEFINITIONJULIA_H