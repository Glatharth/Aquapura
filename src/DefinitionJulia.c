#include "DefinitionJulia.h"
#include <julia.h>
#include <stdio.h>

static jl_function_t *func_avaliar_populacao = NULL;
static jl_function_t *func_fim_geracao = NULL;

static bool checkJuliaError(const char* context) {
    if (jl_exception_occurred()) {
        printf("\n[JULIA ERROR] Falha ao executar: %s\n", context);
        jl_eval_string(
            "if !isnothing(catch_backtrace()) \n"
            "   showerror(stderr, catch_backtrace()[1][1], catch_backtrace()[2]) \n"
            "   println(stderr) \n"
            "end"
        );
        return true; 
    }
    return false;
}

AQUA_EXPORT void C_ReiniciarMundo(int quantidadeDeJogadores) {
    printf("\n[C RECEPTOR] A Julia solicitou o reinicio do mundo com %d jogadores!\n", quantidadeDeJogadores);
    
    extern void resetarGameWorld(int qtdPlayers); 
    resetarGameWorld(quantidadeDeJogadores);
}

bool prepararModelosIA() {
    func_avaliar_populacao = jl_get_function(jl_main_module, "avaliar_populacao");
    func_fim_geracao = jl_get_function(jl_main_module, "fim_de_geracao");
    return true;
}

void obterDecisoesDaPopulacao(double *vetorEstados, int numJogadores, int *vetorAcoes) {
    if (func_avaliar_populacao == NULL) return;

    int tamanhoTotal = numJogadores * 5;

    jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 1);
    jl_array_t *arrayJulia = jl_ptr_to_array_1d(array_type, vetorEstados, tamanhoTotal, 0);

    jl_value_t *retorno = jl_call1(func_avaliar_populacao, (jl_value_t*)arrayJulia);

    if (!checkJuliaError("avaliar_populacao") && retorno != NULL) {
        
        // CORREÇÃO PARA JULIA 1.11+: A macro agora exige o tipo como segundo argumento
        int64_t *acoesJulia = jl_array_data((jl_array_t*)retorno, int64_t);
        
        for(int i = 0; i < numJogadores; i++) {
            vetorAcoes[i] = (int)acoesJulia[i];
        }
    }
}

void notificarFimDeGeracao(double *vetorPontuacoes, int numJogadores) {
    if (func_fim_geracao == NULL) return;

    jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 1);
    jl_array_t *arrayJulia = jl_ptr_to_array_1d(array_type, vetorPontuacoes, numJogadores, 0);

    jl_call1(func_fim_geracao, (jl_value_t*)arrayJulia);
    checkJuliaError("fim_de_geracao");
}