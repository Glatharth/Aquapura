# ==========================================
# PERSISTÊNCIA DOS PESOS NO DISCO (BINÁRIO)
# ==========================================

function salvar_pesos(caminho::String, elites::Vector{RedeNeural})
    try
        open(caminho, "w") do f
            for campeao in elites
                write(f, campeao.W1)
                write(f, campeao.b1)
                write(f, campeao.W2)
                write(f, campeao.b2)
            end
        end
        println("[JULIA] Top $(length(elites)) Mentes Campeãs salvas com sucesso em $caminho")
    catch e
        println("[JULIA ERRO] Falha ao salvar pesos: $e")
    end
end

function carregar_pesos(caminho::String, num_elite::Int)
    if !isfile(caminho)
        println("[JULIA] Nenhum save encontrado. Iniciando do zero.")
        return
    end
    try
        open(caminho, "r") do f
            # Lê os campeões diretamente para as mentes dos primeiros jogadores
            for i in 1:num_elite
                rede = POPULACAO_GLOBAL[i]
                read!(f, rede.W1)
                read!(f, rede.b1)
                read!(f, rede.W2)
                read!(f, rede.b2)
            end
        end
        
        # Replica as mentes dos campeões (com mutações) para o resto da população
        for i in (num_elite + 1):QTD_PLAYERS
            # Escolhe um dos elites aleatoriamente como pai
            pai = POPULACAO_GLOBAL[rand(1:num_elite)]
            POPULACAO_GLOBAL[i] = mutacao(pai, 0.1, 0.2)
        end
        
        println("[JULIA] Mentes campeãs carregadas e replicadas para $QTD_PLAYERS clones!")
    catch e
        println("[JULIA ERRO] Falha ao carregar pesos: $e")
    end
end
