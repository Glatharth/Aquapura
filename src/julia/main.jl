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

function avaliar_estado_arcade(distancia::Float64, energia::Float64)
    println("[JULIA] Recebi os dados do C. Analisando...")
    
    # Processamento "complexo" de IA (brinquedo)
    intensidade_calculada = (distancia * 0.5) + energia
    
    # Se a distância passou de um limite, a Julia ACIONA o C ativamente
    if distancia > 100.0
        println("[JULIA] Condicao critica atingida! Disparando gatilho no C...")
        
        # ccall( (nome_da_funcao_c, biblioteca_ou_exe), Retorno, (Tipos_dos_Argumentos), Valores... )
        # No Windows, usamos "aquapura.exe". No Linux, passariamos C_NULL para o executável atual.
        try
            ccall(
                (:C_DispararEfeitoArcade, "aquapura.exe"), 
                Cvoid, 
                (Int32, Float32), 
                Int32(1), Float32(intensidade_calculada)
            )
        catch e
            println("[JULIA ERRO] Falha ao comunicar com o host C: ", e)
        end
    end
    
    # Após a comunicação bidirecional, devolve um cálculo final para o fluxo normal
    return Float64(intensidade_calculada * 2.0)
end     

function prever_acao_ia(estado_atual::Vector{Float64})
    # Extraindo as variáveis isoladas do vetor que o C mandou
    oxigenio     = estado_atual[1]
    dist_inimigo = estado_atual[2]
    dist_bolha   = estado_atual[3]
    pontuacao    = estado_atual[4]

    # --- LÓGICA DE MACHINE LEARNING (OU REGRAS HARDCODED) ---
    
    # Prioridade 1: Sobrevivência (Oxigênio baixo)
    if oxigenio < 20.0
        if dist_bolha > 0.0 && dist_bolha < 50.0
            # Bolha perto? Vai pra frente buscar!
            return Int64(3) 
        end
    end
    
    # Prioridade 2: Combate
    if dist_inimigo > 0.0 && dist_inimigo < 15.0
        # Inimigo na cara do bot? Captura!
        return Int64(5)
    end
    
    # Ação padrão: Continuar nadando para frente
    return Int64(3)
end