#include <julia.h>
#include <stdio.h>

int initJulia() {
    jl_init();

    jl_eval_string("include(\"src/julia/main.jl\")");

    if (jl_exception_occurred()) {
        println("Erro: Não foi possível carregar ou executar o arquivo main.jl.");
        jl_eval_string("println(sprint(showerror, catch_backtrace()[1][1]))");
        jl_atexit_hook(1);
        return 1;
    }

    // Busca e executa a função
    jl_function_t *func = jl_get_function(jl_main_module, "print");

    if (func != NULL) {
        jl_call0(func);
    } else {
        println("Erro: Função 'print' não encontrada no main.jl!");
    }

    jl_atexit_hook(0);
    return 0;
}