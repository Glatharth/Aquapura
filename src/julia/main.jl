function print()
    println("Hello, World! --Ass: Julia")
end

function testar_comunicacao(cenario::Int64)
    println("\n--- [JULIA] Iniciando cenario de teste: $cenario ---")

    if cenario == 1
        println("[JULIA] Cenario 1: Tudo certo. Retornando um Int64.")
        return Int64(42)

    elseif cenario == 2
        println("[JULIA] Cenario 2: Retornando uma String (vai forcar falha de tipagem no C).")
        return "Eu sou uma string, nao um numero!"

    elseif cenario == 3
        println("[JULIA] Cenario 3: Gerando um erro nativo da Julia (Crash intencional).")
        error("Este e um erro intencional gerado na Julia!")

    elseif cenario == 4
        println("[JULIA] Cenario 4: Chamando a funcaoForte() do C...")
        try
            # O ccall procura a função "funcaoForte" compilada no executável host
            ccall(:funcaoForte, Cvoid, ())
            println("[JULIA] funcaoForte executada sem erros no catch.")
        catch e
            println("[JULIA] Capturei um erro vindo do C: ", e)
        end
        return Int64(99)

    else
        println("[JULIA] Cenario desconhecido.")
        return Int64(0)
    end
end