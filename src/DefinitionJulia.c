#include "DefinitionJulia.h"
#include <julia.h>
#include <stdio.h>

// Cache para não buscar a função a cada frame
static jl_function_t *func_prever_acao = NULL;
static jl_function_t *func_teste_bidirecional = NULL;

// --- HELPERS PRIVADOS DE TRATAMENTO DE ERRO ---

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

static bool validateJuliaType(jl_value_t *value, jl_datatype_t *expectedType, const char* varName) {
    if (value == NULL) {
        printf("[TYPE ERROR] A variavel '%s' retornou NULL.\n", varName);
        return false;
    }
    if (!jl_isa(value, (jl_value_t*)expectedType)) {
        printf("[TYPE ERROR] Incompatibilidade na variavel '%s'. Tipo inesperado.\n", varName);
        return false;
    }
    return true;
}

void throwJuliaErrorFromC(const char* errorMessage) {
    printf("[C ERROR] Lancando excecao para a Julia: %s\n", errorMessage);
    jl_error(errorMessage); 
}

// --- IMPLEMENTAÇÃO DA FUNÇÃO C (CHAMADA PELA JULIA) ---

AQUA_EXPORT void C_DispararEfeitoArcade(int tipoEfeito, float intensidade) {
    printf("\n[C RECEPTOR] A Julia me chamou de volta!\n");
    printf("[C RECEPTOR] Comando recebido -> Efeito ID: %d | Intensidade: %.2f\n", tipoEfeito, intensidade);
    
    if (tipoEfeito == 1) {
        printf("[C RECEPTOR] Acao: Spawna um obstaculo massivo na tela!\n\n");
    } else {
        printf("[C RECEPTOR] Acao: Toca som de bonus!\n\n");
    }
}

// --- IMPLEMENTAÇÃO DA API PARA O JOGO ---

bool prepararModelosIA() {
    // Note que agora precisamos buscar a função real que o jogo vai usar também
    func_prever_acao = jl_get_function(jl_main_module, "prever_acao_ia");
    func_teste_bidirecional = jl_get_function(jl_main_module, "avaliar_estado_arcade");
    return true;
}

float testarBidirecionalidade(float distanciaPercorrida, float energiaAtual) {
    if (func_teste_bidirecional == NULL) return 0.0f;

    printf("[C EMISSOR] Enviando dados para a Julia: Distancia %.2f, Energia %.2f\n", distanciaPercorrida, energiaAtual);

    jl_value_t *arg1 = jl_box_float64((double)distanciaPercorrida);
    jl_value_t *arg2 = jl_box_float64((double)energiaAtual);

    jl_value_t *retorno = jl_call2(func_teste_bidirecional, arg1, arg2);

    float resultadoFinal = 0.0f;
    if (jl_isa(retorno, (jl_value_t*)jl_float64_type)) {
        resultadoFinal = (float)jl_unbox_float64(retorno);
        printf("[C EMISSOR] Resultado final devolvido pela Julia: %.2f\n", resultadoFinal);
    }

    return resultadoFinal;
}

AcaoBot obterDecisaoDaIA(EstadoAquapura estado) {
    if (func_prever_acao == NULL) return ACAO_ESPERAR;

    double vetorEstado[4] = {
        (double)estado.oxigenio,
        (double)estado.distInimigo,
        (double)estado.distBolha,
        (double)estado.pontuacao
    };

    // Construção correta do tipo de array na API da Julia: Vector{Float64}
    jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_float64_type, 1);
    
    // Passaporte Zero-Copy
    jl_array_t *arrayJulia = jl_ptr_to_array_1d(array_type, vetorEstado, 4, 0);

    jl_value_t *retorno = jl_call1(func_prever_acao, (jl_value_t*)arrayJulia);

    if (checkJuliaError("Execucao da IA do Aquapura")) {
        return ACAO_ESPERAR; 
    }

    if (validateJuliaType(retorno, jl_int64_type, "decisao_ia")) {
        int decisao = (int)jl_unbox_int64(retorno);
        if (decisao >= 0 && decisao <= 5) {
            return (AcaoBot)decisao;
        }
    }

    return ACAO_ESPERAR;
}