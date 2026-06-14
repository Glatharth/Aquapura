include("Telemetry.jl")
include("ConfigIA.jl")
using .ConfigIA
include("PPO.jl")
using .PPO
using Statistics
using Sockets
using BSON

# ==========================================
# INICIALIZAÇÃO E CARREGAMENTO DA IA
# ==========================================

# Tenta carregar pesos existentes, se não achar, cria um novo
function inicializar_agente()
    agente_novo = PPO.PPOAgent(ConfigIA.NUM_ENTRADAS, 
                               lr=ConfigIA.LEARNING_RATE,
                               gamma=ConfigIA.GAMMA,
                               epochs=ConfigIA.EPOCHS,
                               batch_size=ConfigIA.BATCH_SIZE,
                               c_entropy=ConfigIA.ENTROPY_COEF)
    
    if isfile(ConfigIA.ARQUIVO_PESOS)
        try
            dados = BSON.load(ConfigIA.ARQUIVO_PESOS)
            agente_carregado = dados[:agente]
            
            # Move o modelo salvo (que poderia estar na CPU) para o dispositivo correto (GPU/CPU)
            agente_novo.model = agente_carregado.model |> PPO.DEVICE
            
            # Reinicia o estado do otimizador para a arquitetura de memória do novo dispositivo
            agente_novo.opt_state = PPO.Optimisers.setup(PPO.Optimisers.Adam(ConfigIA.LEARNING_RATE), agente_novo.model)
            
            println("[IA] Pesos carregados de $(ConfigIA.ARQUIVO_PESOS) e transferidos para o processamento nativo!")
        catch e
            println("[IA] Erro ao carregar pesos (", e, "). Usando IA crua recém-criada...")
        end
    else
        println("[IA] Nenhum peso salvo encontrado. Criando nova IA do zero na placa padrão...")
    end
    
    return agente_novo
end

const AGENTE = inicializar_agente()

# Tracker para punição de inatividade e spam
const CACHE_INATIVIDADE = zeros(Int, ConfigIA.QTD_PLAYERS)
const LIMITE_INATIVIDADE = ConfigIA.LIMITE_INATIVIDADE

# Lock para salvar transições de forma segura usando multi-threading
const MEMORY_LOCK = ReentrantLock()

# Cache de estados por jogador
const CACHE_STATE = [zeros(Float32, ConfigIA.NUM_ENTRADAS) for _ in 1:ConfigIA.QTD_PLAYERS]
const CACHE_MOVE = zeros(Int, ConfigIA.QTD_PLAYERS)
const CACHE_CAP = fill(Int64(1), ConfigIA.QTD_PLAYERS)
const CACHE_LOGPROB = zeros(Float32, ConfigIA.QTD_PLAYERS)
const CACHE_VALUE = zeros(Float32, ConfigIA.QTD_PLAYERS)
const CACHE_PONTOS = zeros(Float32, ConfigIA.QTD_PLAYERS)
const CACHE_WAS_ALIVE = fill(false, ConfigIA.QTD_PLAYERS)
const HISTORICO_PONTOS = Float32[]
const CACHE_OXI = zeros(Float32, ConfigIA.QTD_PLAYERS)

# Estatísticas Visuais
global geracao_atual = 1
global step_atual = 0

# ==========================================
# EXTRAÇÃO DO VETOR DE ESTADOS (OTIMIZADO)
# ==========================================
@inline function extrair_entidades(estados_unificados::Vector{Float64})
    idx = 1
    num_npcs = Int(estados_unificados[idx])
    idx += 1
    
    lixos, peixes, bolhas = Tuple{Float32, Float32}[], Tuple{Float32, Float32}[], Tuple{Float32, Float32}[]
    sizehint!(lixos, 20); sizehint!(peixes, 20); sizehint!(bolhas, 5)
    
    @inbounds for _ in 1:num_npcs
        tipo = estados_unificados[idx]
        nx = Float32(estados_unificados[idx+1])
        ny = Float32(estados_unificados[idx+2])
        idx += 4 # pula speed_x
        
        if tipo == 1.0 push!(lixos, (nx, ny))
        elseif tipo == 2.0 push!(peixes, (nx, ny))
        elseif tipo == 3.0 push!(bolhas, (nx, ny))
        end
    end
    return idx, lixos, peixes, bolhas
end

@inline function preencher_top3!(vetor_entidades::Vector{Tuple{Float32, Float32}}, offset::Int, bot_x::Float32, bot_y::Float32, input_vec::Vector{Float32})
    sort!(vetor_entidades, by = p -> (p[1]-bot_x)^2 + (p[2]-bot_y)^2)
    
    # [Apresentação] Metaprogramação (Macros): A macro `@inbounds` ordena ao compilador do Julia para 
    # desativar todas as travas de segurança de "Out of Bounds" na leitura de arrays dentro do loop.
    # Como já foi garantido a matemática no código, isso injeta uma performance na leitura das entidades.
    @inbounds for j in 1:3
        if j <= length(vetor_entidades)
            ex, ey = vetor_entidades[j]
            dx = ex - bot_x
            dy = ey - bot_y
            dist = sqrt(dx^2 + dy^2)
            input_vec[offset + (j-1)*3 + 1] = dx / 1000.0f0
            input_vec[offset + (j-1)*3 + 2] = dy / 1000.0f0
            input_vec[offset + (j-1)*3 + 3] = dist / 1000.0f0
        else
            input_vec[offset + (j-1)*3 + 1] = 0.0f0
            input_vec[offset + (j-1)*3 + 2] = 0.0f0
            input_vec[offset + (j-1)*3 + 3] = 1.0f0
        end
    end
end

# ==========================================
# FUNÇÃO PRINCIPAL DE AVALIAÇÃO CHAMADA PELO C
# ==========================================
# [Apresentação] Tipagem Dinâmica JIT: Aqui eu declaro o tipo exato da entrada (Vector{Float64}) e saída (Vector{Int64}).
# Ao contrário do Python, o Julia compila essas tipagens (JIT) e gera um código em Assembly super otimizado instantaneamente,
# garantindo que o C consiga ler e escrever nessa função sem conversões lentas.
function avaliar_populacao(estados_unificados::Vector{Float64})::Vector{Int64}
    try
        global step_atual
        step_atual += 1
        
        idx_jogadores, lixos, peixes, bolhas = extrair_entidades(estados_unificados)
        
        num_jogadores = Int(estados_unificados[idx_jogadores])
        idx_base = idx_jogadores + 1
        
        acoes_de_retorno = fill(Int64(0), num_jogadores)
        
        # [Apresentação] Metaprogramação (Macros): A macro `@threads` reescreve esse laço 'for' antes da compilação.
        # Ela automaticamente pega as N IAs jogando e distribui em threads nativas do processador rodando em paralelo.
        # É por isso que o cálculo das IAs ocorre simultaneamente sem engasgar a renderização visual do jogo.
        Threads.@threads for i in 1:num_jogadores
            # Multiplicador 4 porque cada jogador ocupa 4 posições (x, y, oxi, colisao)
            offset_j = idx_base + (i - 1) * 4
            bot_x = Float32(estados_unificados[offset_j])
            bot_y = Float32(estados_unificados[offset_j+1])
            oxigenio = Float32(estados_unificados[offset_j+2])
            colisao = Float32(estados_unificados[offset_j+3])
            
            # Ignora peixes mortos
            if bot_x == -1.0f0
                CACHE_OXI[i] = 100.0f0
                CACHE_WAS_ALIVE[i] = false
                # A pontuação real é enviada pro histórico apenas quando o PPO fecha o ciclo de vida
                continue
            end
            
            # --- REWARD CALCULATION ---
            # Process transition from previous frame for PPO memory buffer
            if step_atual > 1 && CACHE_WAS_ALIVE[i]
                reward = -0.01f0 # Time penalty
                done = false
                suicide = false
                
                # Dense reward for moving towards nearest garbage and maintaining oxygen levels
                dist_lixo = 1.0f0
                if length(lixos) > 0
                    dx = lixos[1][1] - bot_x
                    dy = lixos[1][2] - bot_y
                    dist_lixo = sqrt(dx^2 + dy^2) / 1000.0f0
                end
                
                reward += (1.0f0 - min(dist_lixo, 1.0f0)) * 0.02f0
                reward += (oxigenio / 100.0f0) * 0.01f0
                
                # Boundary collision penalty
                if bot_x < 50.0f0 || bot_x > 950.0f0 || bot_y > 750.0f0
                    reward -= 0.05f0
                end
                
                # False positive capture penalty
                if CACHE_CAP[i] == 2 && colisao == 0.0f0
                    reward -= 0.002f0
                end
                
                # Entity collision resolution
                if colisao < 0.0f0
                    reward -= 1.0f0
                    CACHE_PONTOS[i] -= 1.0f0
                elseif colisao > 0.0f0
                    reward += 1.0f0
                    CACHE_INATIVIDADE[i] = 0
                    CACHE_PONTOS[i] += 1.0f0
                end
                
                CACHE_INATIVIDADE[i] += 1
                if CACHE_INATIVIDADE[i] >= LIMITE_INATIVIDADE
                    suicide = true
                    reward -= 1.0f0
                end
                
                # Termination conditions
                if oxigenio <= 0.0f0 || oxigenio < CACHE_OXI[i] - 10.0f0 || suicide
                    reward -= 1.0f0
                    done = true
                    CACHE_INATIVIDADE[i] = 0
                    
                    # [Apresentação] First-Class Functions (Do-Blocks): Esse 'do' injeta todo o bloco interno como uma função anônima
                    # dentro da função 'lock()'. Isso é uma construção elegante do Julia (passar blocos de código como variáveis).
                    lock(MEMORY_LOCK) do
                        push!(HISTORICO_PONTOS, CACHE_PONTOS[i])
                    end
                    CACHE_PONTOS[i] = 0.0f0
                end
                
                # Store memory transition
                lock(MEMORY_LOCK) do
                    PPO.store_transition!(AGENTE.memory, CACHE_STATE[i], CACHE_MOVE[i], CACHE_CAP[i], CACHE_LOGPROB[i], reward, CACHE_VALUE[i], done)
                end
            end
            CACHE_OXI[i] = oxigenio
            
            # --- STATE UPDATE ---
            input_vec = CACHE_STATE[i]
            input_vec[1] = bot_x / 1000.0f0
            input_vec[2] = bot_y / 1000.0f0
            input_vec[3] = oxigenio / 100.0f0
            
            preencher_top3!(lixos, 3, bot_x, bot_y, input_vec)
            preencher_top3!(peixes, 12, bot_x, bot_y, input_vec)
            preencher_top3!(bolhas, 21, bot_x, bot_y, input_vec)
            
            # --- AGENT INFERENCE ---
            move_a, cap_a, logp, val = PPO.sample_action(AGENTE, input_vec)
            
            CACHE_MOVE[i] = move_a
            CACHE_CAP[i] = cap_a
            CACHE_LOGPROB[i] = logp
            CACHE_VALUE[i] = val
            
            # Serialize actions for C interaction
            c_move = move_a - 1
            c_cap = (cap_a == 2) ? 8 : 0
            c_suicide = (CACHE_INATIVIDADE[i] >= ConfigIA.LIMITE_INATIVIDADE) ? 16 : 0
            
            acoes_de_retorno[i] = Int64(c_move | c_cap | c_suicide)
            CACHE_WAS_ALIVE[i] = true
        end
        
        # --- MODEL OPTIMIZATION ---
        if step_atual >= ConfigIA.ROLLOUT_STEPS
            buffer_len = length(AGENTE.memory.rewards)
            println("[PPO] Optimization Triggered (Rollout of $(ConfigIA.ROLLOUT_STEPS) frames) | Buffer: $buffer_len transitions")
            
            if buffer_len > 0
                melhor_fit = maximum(AGENTE.memory.rewards)
                medio_fit = mean(AGENTE.memory.rewards)
                
                todos_pontos = vcat(CACHE_PONTOS[1:num_jogadores], HISTORICO_PONTOS)
                melhor_jogo = isempty(todos_pontos) ? 0.0f0 : maximum(todos_pontos)
                medio_jogo = isempty(todos_pontos) ? 0.0f0 : mean(todos_pontos)
                empty!(HISTORICO_PONTOS)
                
                # Dispatch UDP Telemetry
                try
                    sock = UDPSocket()
                    send(sock, ip"127.0.0.1", 9876, "$geracao_atual,$melhor_fit,$medio_fit,$melhor_jogo,$medio_jogo")
                    close(sock)
                catch e
                    # Non-blocking if dashboard socket is unavailable
                end
            end
            
            PPO.update_agent!(AGENTE)
            
            # Persist model weights periodically
            if geracao_atual % 10 == 0
                BSON.bson(ConfigIA.ARQUIVO_PESOS, agente=AGENTE)
                println("[IA] Weights successfully saved to $(ConfigIA.ARQUIVO_PESOS) (Generation $geracao_atual)")
            end
            
            step_atual = 0
            global geracao_atual += 1
        end
        
        return acoes_de_retorno
    catch e
        println(stderr, "========= JULIA INTERNAL ERROR =========")
        showerror(stderr, e, catch_backtrace())
        println(stderr, "\n========================================")
        return Int64[]
    end
end

function fim_de_geracao()
    # Esta função era usada pelo codigo antigo. Não fazemos nada aqui, o PPO atualiza por steps.
end