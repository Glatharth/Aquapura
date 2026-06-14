module PPO

using Flux
using Optimisers
using Distributions
using StatsBase
using Random
using Statistics

using Flux
using Optimisers
using Distributions
using StatsBase
using Random
using Statistics

# Detecção dinâmica da GPU
function get_device()
    try
        @eval using CUDA
        if CUDA.functional()
            println("[IA] Rodando modelo em: GPU (NVIDIA CUDA)")
            return Flux.gpu
        end
    catch
    end
    try
        @eval using AMDGPU
        if AMDGPU.functional()
            println("[IA] Rodando modelo em: GPU (AMD ROCm)")
            return Flux.gpu
        end
    catch
    end
    println("[IA] Rodando modelo em: CPU (Padrão)")
    return Flux.cpu
end

const DEVICE = get_device()

export ActorCritic, PPOAgent, PPOMemory, store_transition!, update_agent!, sample_action, get_action_logprob, DEVICE

# ==========================================
# 1. ARQUITETURA DA REDE NEURAL (Flux.jl)
# ==========================================

# O Agente usa múltiplas redes (ou redes com base compartilhada)
# Para jogos simples, separar Ator e Crítico melhora a estabilidade.
struct ActorCritic
    base::Chain
    actor_move::Chain
    actor_capture::Chain
    critic::Chain
end

# Dizemos ao Flux que ActorCritic é um modelo cujos parâmetros podem ser treinados
Flux.@layer ActorCritic

function ActorCritic(num_inputs::Int)
    base = Chain(
        Dense(num_inputs, 128, relu),
        Dense(128, 128, relu)
    )
    
    # Ator de Movimento: 5 saídas (Softmax)
    actor_move = Chain(Dense(128, 5))
    
    # Ator de Captura: 2 saídas (Softmax para Sim/Não) - Mais estável que Sigmoid puro com Flux logitcrossentropy
    actor_capture = Chain(Dense(128, 2))
    
    # Crítico: 1 saída (Valor esperado do estado)
    critic = Chain(Dense(128, 1))
    
    return ActorCritic(base, actor_move, actor_capture, critic)
end

function (ac::ActorCritic)(state::AbstractArray)
    h = ac.base(state)
    move_logits = ac.actor_move(h)
    cap_logits = ac.actor_capture(h)
    value = ac.critic(h)
    return move_logits, cap_logits, value
end


# ==========================================
# 2. MEMÓRIA DO ROLLOUT (Buffer)
# ==========================================
mutable struct PPOMemory
    states::Vector{Vector{Float32}}
    move_actions::Vector{Int}
    cap_actions::Vector{Int}
    logprobs::Vector{Float32}
    rewards::Vector{Float32}
    values::Vector{Float32}
    dones::Vector{Bool}
end

PPOMemory() = PPOMemory(Vector{Float32}[], Int[], Int[], Float32[], Float32[], Float32[], Bool[])

function clear_memory!(mem::PPOMemory)
    empty!(mem.states)
    empty!(mem.move_actions)
    empty!(mem.cap_actions)
    empty!(mem.logprobs)
    empty!(mem.rewards)
    empty!(mem.values)
    empty!(mem.dones)
end

function store_transition!(mem::PPOMemory, state, move_a, cap_a, logprob, reward, value, done)
    push!(mem.states, copy(state))
    push!(mem.move_actions, move_a)
    push!(mem.cap_actions, cap_a)
    push!(mem.logprobs, logprob)
    push!(mem.rewards, reward)
    push!(mem.values, value)
    push!(mem.dones, done)
end

# ==========================================
# 3. AGENTE E SELEÇÃO DE AÇÕES
# ==========================================
mutable struct PPOAgent
    model::ActorCritic
    opt_state::NamedTuple
    memory::PPOMemory
    
    # Hiperparâmetros
    gamma::Float32      # Desconto
    lambda::Float32     # GAE smoothing
    clip_ratio::Float32 # PPO clipping
    c_value::Float32    # Peso da perda do Crítico
    c_entropy::Float32  # Peso da Entropia (exploração)
    epochs::Int         # Quantas vezes otimizar na mesma memória
    batch_size::Int     # Tamanho do Minibatch
end

function PPOAgent(num_inputs::Int; lr=3e-4, gamma=0.99, lambda=0.95, clip_ratio=0.2, c_value=0.5, c_entropy=0.01, epochs=4, batch_size=64)
    model = ActorCritic(num_inputs) |> DEVICE
    opt = Optimisers.setup(Optimisers.Adam(lr), model)
    mem = PPOMemory()
    return PPOAgent(model, opt, mem, gamma, lambda, clip_ratio, c_value, c_entropy, epochs, batch_size)
end

function sample_action(agent::PPOAgent, state::Vector{Float32})
    # Flux trabalha melhor com matrizes. Movemos o estado para a GPU (ou CPU) dinamicamente.
    s_mat = reshape(state, length(state), 1) |> DEVICE
    
    move_logits, cap_logits, value = agent.model(s_mat)
    
    # Trazemos de volta para a CPU para usar Distributions.jl (Amostragem estatística)
    move_logits = move_logits |> Flux.cpu
    cap_logits = cap_logits |> Flux.cpu
    value = value |> Flux.cpu
    
    # Softmax para obter probabilidades
    move_probs = Flux.softmax(move_logits, dims=1) |> vec
    cap_probs = Flux.softmax(cap_logits, dims=1) |> vec
    
    # Sampling categórico
    move_dist = Categorical(move_probs)
    cap_dist = Categorical(cap_probs)
    
    move_a = rand(move_dist)
    cap_a = rand(cap_dist)
    
    # Logprob conjunta = log(P(move)) + log(P(cap))
    logp = logpdf(move_dist, move_a) + logpdf(cap_dist, cap_a)
    
    return move_a, cap_a, Float32(logp), Float32(value[1])
end


# ==========================================
# 4. ALGORITMO PPO (TREINAMENTO)
# ==========================================

function compute_gae(rewards, values, dones, gamma, lambda)
    T = length(rewards)
    advantages = zeros(Float32, T)
    returns = zeros(Float32, T)
    
    last_gae = 0.0f0
    for t in T:-1:1
        # Se for o último frame da transição, o próximo valor é 0 (ou aproximação se truncado, mas vamos simplificar para 0 se done)
        next_val = (t == T) ? 0.0f0 : values[t+1]
        next_non_terminal = (t == T) ? 0.0f0 : 1.0f0 - Float32(dones[t])
        
        delta = rewards[t] + gamma * next_val * next_non_terminal - values[t]
        last_gae = delta + gamma * lambda * next_non_terminal * last_gae
        
        advantages[t] = last_gae
        returns[t] = advantages[t] + values[t]
    end
    
    # Normalizar vantagens (Estabiliza enormemente o treino)
    adv_mean = mean(advantages)
    adv_std = std(advantages) + 1f-8
    advantages = (advantages .- adv_mean) ./ adv_std
    
    return advantages, returns
end

function update_agent!(agent::PPOAgent)
    mem = agent.memory
    T = length(mem.rewards)
    if T == 0 return end
    
    # Converte arrays do buffer e já manda os vetores de alvo para a GPU
    states_mat = hcat(mem.states...) |> DEVICE
    old_logprobs = Float32.(mem.logprobs) |> DEVICE
    
    # Calcula GAE e Retornos (Na CPU, operações leves)
    advantages, returns = compute_gae(mem.rewards, mem.values, mem.dones, agent.gamma, agent.lambda)
    
    # Transfere vantagens e retornos para a GPU
    adv_full = advantages |> DEVICE
    ret_full = returns |> DEVICE
    
    # Ações são mantidas na CPU temporariamente para gerar as máscaras
    move_actions_cpu = mem.move_actions
    cap_actions_cpu = mem.cap_actions
    
    # Configura batches
    indices = collect(1:T)
    
    for epoch in 1:agent.epochs
        shuffle!(indices)
        for i in 1:agent.batch_size:T
            batch_idx = indices[i:min(i+agent.batch_size-1, T)]
            
            s_batch = states_mat[:, batch_idx]
            adv_batch = adv_full[batch_idx]
            ret_batch = ret_full[batch_idx]
            old_lp_batch = old_logprobs[batch_idx]
            
            # Ações mantidas na CPU
            move_a_batch = move_actions_cpu[batch_idx]
            cap_a_batch = cap_actions_cpu[batch_idx]
            
            # Cálculo de Gradientes (Backpropagation na CPU via Zygote)
            grads = Flux.gradient(agent.model) do m
                m_logits, c_logits, vals = m(s_batch)
                
                # Novas probabilidades
                m_logprobs = Flux.logsoftmax(m_logits, dims=1)
                c_logprobs = Flux.logsoftmax(c_logits, dims=1)
                
                # [Apresentação] Vetorização Nativa (Broadcasting): Em Julia, o uso do ponto '.' antes das operações (como '.+') 
                # ou chamadas de função (como 'CartesianIndex.()') instrui o processador a realizar cálculos matemáticos
                # em blocos simultâneos (SIMD) diretamente nos arrays inteiros. É extremamente performático e substitui laços 'for' complexos.
                indices_m = CartesianIndex.(move_a_batch, 1:length(batch_idx))
                indices_c = CartesianIndex.(cap_a_batch, 1:length(batch_idx))
                new_lp_batch = m_logprobs[indices_m] .+ c_logprobs[indices_c]
                
                # Calcula entropia -sum(p * log(p)) para incentivar exploração (Vetorizado)
                p_m = exp.(m_logprobs)
                p_c = exp.(c_logprobs)
                entropy_m = -sum(p_m .* m_logprobs, dims=1)
                entropy_c = -sum(p_c .* c_logprobs, dims=1)
                entropy = mean(entropy_m .+ entropy_c)
                
                # Razão de PPO (r_theta)
                ratios = exp.(new_lp_batch .- old_lp_batch)
                
                # Clipped Surrogate Loss
                surr1 = ratios .* adv_batch
                surr2 = clamp.(ratios, 1.0f0 - agent.clip_ratio, 1.0f0 + agent.clip_ratio) .* adv_batch
                actor_loss = -mean(min.(surr1, surr2))
                
                # Critic Loss (MSE)
                critic_loss = mean((vec(vals) .- ret_batch).^2)
                
                # Perda Total
                total_loss = actor_loss + agent.c_value * critic_loss - agent.c_entropy * entropy
                return total_loss
            end
            
            # Aplica gradientes e atualiza pesos da rede
            Optimisers.update!(agent.opt_state, agent.model, grads[1])
        end
    end
    
    # Limpa buffer para próximo rollout
    clear_memory!(agent.memory)
end

end # module
