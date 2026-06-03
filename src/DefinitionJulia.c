#include "DefinitionJulia.h"
#include <julia.h> // A biblioteca da Julia fica restrita e escondida aqui
#include <stdio.h>

// Cache para não buscar a função a cada frame (vital para performance)
static jl_function_t *func_prever_acao = NULL;

// --- HELPERS PRIVADOS DE TRATAMENTO DE ERRO ---
// Usamos "static" para que não conflitem com outras partes do código C

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

// --- IMPLEMENTAÇÃO DA API PARA O JOGO ---

bool prepararModelosIA() {
    // Procura a função principal de ML no módulo raiz da Julia
    func_prever_acao = jl_get_function(jl_main_module, "prever_acao_ia");

    if (func_prever_acao == NULL) {
        printf("[DEFINITION ERRO] Funcao 'prever_acao_ia' nao encontrada nos scripts!\n");
        return false;
    }

    printf("[DEFINITION] Modelos de IA cacheados e prontos para uso.\n");
    return true;
}

AcaoBot preverAcaoBot(EstadoDoJogo estadoAtual) {
    if (func_prever_acao == NULL) return ACAO_NENHUMA;

    // 1. Empacota a struct de C para tipos que a Julia entenda
    jl_value_t *argX    = jl_box_float64((double)estadoAtual.posicaoX);
    jl_value_t *argY    = jl_box_float64((double)estadoAtual.posicaoY);
    jl_value_t *argDist = jl_box_float64((double)estadoAtual.distanciaProximoObstaculo);

    // 2. Executa a função (passando 3 argumentos)
    jl_value_t *retorno = jl_call3(func_prever_acao, argX, argY, argDist);

    // 3. Valida se a Julia explodiu durante o cálculo
    if (checkJuliaError("preverAcaoBot")) {
        return ACAO_NENHUMA;
    }

    // 4. Valida e extrai o resultado
    if (validateJuliaType(retorno, jl_int64_type, "retorno de prever_acao_ia")) {
        int decisao = (int)jl_unbox_int64(retorno);
        
        // Mapeia o inteiro seguro para o Enum do jogo
        if (decisao >= 0 && decisao <= 2) {
            return (AcaoBot)decisao;
        }
    }

    return ACAO_NENHUMA;
}