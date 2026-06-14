#include "ConfigJulia.h"
#include <julia.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

// Função auxiliar para verificar se a string termina com .jl
bool endsWith(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    size_t lenStr = strlen(str);
    size_t lenSuffix = strlen(suffix);
    if (lenSuffix > lenStr) return false;
    return strncmp(str + lenStr - lenSuffix, suffix, lenSuffix) == 0;
}

// Função que varre a pasta e executa o include() na Julia
void loadJuliaScriptsFromDirectory(const char *folderPath) {
    DIR *dir = opendir(folderPath);
    if (dir == NULL) {
        printf("[CONFIG ERRO] Nao foi possivel abrir o diretorio '%s'.\n", folderPath);
        return;
    }

    printf("[CONFIG] Iniciando leitura da pasta: %s\n", folderPath);
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (endsWith(entry->d_name, ".jl")) {
            char fullPath[512];
            char juliaCommand[600];

            snprintf(fullPath, sizeof(fullPath), "%s/%s", folderPath, entry->d_name);
            snprintf(juliaCommand, sizeof(juliaCommand), "include(\"%s\")", fullPath);

            printf("[CONFIG] Carregando script: %s\n", fullPath);
            jl_eval_string(juliaCommand);

            if (jl_exception_occurred()) {
                printf("[CONFIG ERRO] Falha ao carregar '%s':\n", entry->d_name);
                jl_eval_string("println(sprint(showerror, catch_backtrace()[1][1]))");
            }
        }
    }
    closedir(dir);
}

int juliaInit() {
    printf("[CONFIG] Inicializando VM da Julia...\n");
    jl_init();
    
    // Ativa o ambiente local (Project.toml) para ter acesso ao Flux e outras dependências
    printf("[CONFIG] Ativando ambiente local Julia...\n");
    jl_eval_string("using Pkg; Pkg.activate(\".\")");

    // Carrega apenas o script principal, que já importa suas dependências
    printf("[CONFIG] Carregando script principal...\n");
    jl_eval_string("include(\"src/julia/IaPlayer.jl\")");

    if (jl_exception_occurred()) {
        printf("[CONFIG ERRO] Nao foi possivel carregar ou executar os arquivos Julia.\n");
        jl_eval_string("println(sprint(showerror, catch_backtrace()[1][1]))");
        jl_atexit_hook(1);
        return 1;
    }

    return 0;
}

void juliaClose() {
    printf("[CONFIG] Encerrando VM da Julia...\n");
    jl_atexit_hook(0);
}