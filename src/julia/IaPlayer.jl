@enum AcaoBot::Int64 begin
    ACAO_ESPERAR  = 0
    ACAO_CIMA     = 1
    ACAO_BAIXO    = 2
    ACAO_FRENTE   = 3
    ACAO_TRAS     = 4
    ACAO_CAPTURAR = 5
end

# 1. AVALIAÇÃO DA REDE NEURAL (Processa todos os clones em PARALELO)
function avaliar_populacao(estados_unificados::Vector{Float64})::Vector{Int64}
    num_variaveis = 5 
    num_jogadores = div(length(estados_unificados), num_variaveis)
    
    acoes_de_retorno = zeros(Int64, num_jogadores)
    
    # A MÁGICA ACONTECE AQUI: O "@threads" divide o loop entre os núcleos da sua CPU!
    Threads.@threads for i in 1:num_jogadores
        
        # Descobre a posição dos dados deste clone no array gigante
        idx = (i - 1) * num_variaveis
        
        oxigenio     = estados_unificados[idx + 1]
        dist_inimigo = estados_unificados[idx + 2]
        dist_bolha   = estados_unificados[idx + 3]
        pontuacao    = estados_unificados[idx + 4]
        vivo         = estados_unificados[idx + 5]
        
        # Se o clone já morreu, ele não faz nada
        if vivo == 0.0
            acoes_de_retorno[i] = Int64(ACAO_ESPERAR)
            continue 
        end
        
        # ==========================================
        # CÉREBRO TÁTICO (Aplicado a cada clone)
        # ==========================================
        
        # 1. Pânico de Oxigênio
        if oxigenio < 35.0
            if dist_bolha > 0.0 && dist_bolha < 100.0
                acoes_de_retorno[i] = Int64(ACAO_FRENTE)
            else
                acoes_de_retorno[i] = Int64(ACAO_CIMA)
            end
            continue
        end

        # 2. Combate Tático
        if dist_inimigo != 1000.0 
            if dist_inimigo <= 35.0
                acoes_de_retorno[i] = Int64(ACAO_CAPTURAR)
                continue
            end
            if dist_inimigo > 35.0 && dist_inimigo <= 80.0
                acoes_de_retorno[i] = Int64(ACAO_TRAS)
                continue
            end
            if dist_inimigo > 80.0 && dist_inimigo <= 150.0
                acoes_de_retorno[i] = Int64(ACAO_ESPERAR)
                continue
            end
            if dist_inimigo > 150.0
                acoes_de_retorno[i] = Int64(ACAO_FRENTE)
                continue
            end
        end

        # 3. Exploração / Reposicionamento
        if oxigenio > 90.0
            sorteio = rand()
            if sorteio < 0.5
                acoes_de_retorno[i] = Int64(ACAO_BAIXO)
            elseif sorteio < 0.8
                acoes_de_retorno[i] = Int64(ACAO_TRAS)
            else
                acoes_de_retorno[i] = Int64(ACAO_ESPERAR)
            end
            continue
        end

        sorteio_nado = rand()
        if sorteio_nado < 0.1
            acoes_de_retorno[i] = Int64(ACAO_CIMA)
        elseif sorteio_nado < 0.2
            acoes_de_retorno[i] = Int64(ACAO_BAIXO)
        elseif sorteio_nado < 0.8
            acoes_de_retorno[i] = Int64(ACAO_TRAS) 
        else
            acoes_de_retorno[i] = Int64(ACAO_FRENTE) 
        end
    end
    
    return acoes_de_retorno
end

# 2. O DEUS DA MATRIX (Ocorre quando todos os clones morrem no C)
function fim_de_geracao(pontuacoes::Vector{Float64})
    println("\n[JULIA] === FIM DE GERACAO ===")
    println("[JULIA] Processando o score de ", length(pontuacoes), " jogadores.")
    
    # Salva o arquivo CSV
    try
        open("historico_treinamento.csv", "a") do arquivo
            linha = join(pontuacoes, ",")
            println(arquivo, linha)
        end
        println("[JULIA] Pontuacoes salvas em historico_treinamento.csv")
    catch e
        println("[JULIA ERRO] Nao foi possivel salvar o CSV: ", e)
    end
    
    # --- Aqui entrará o Algoritmo Genético do seu Perceptron no futuro ---
    
    quantidade_novos_players = 50 
    println("[JULIA] Reiniciando o mundo com $quantidade_novos_players clones...\n")
    
    try
        # Reset compatível nativamente com Linux (sem exigir "aquapura.exe")
        ccall(:C_ReiniciarMundo, Cvoid, (Int32,), Int32(quantidade_novos_players))
    catch e
        println("[JULIA ERRO FATAL] Falha ao acionar reset no C: ", e)
    end
end