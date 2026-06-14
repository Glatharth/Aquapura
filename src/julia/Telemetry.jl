module Telemetry

using Sockets

# Instância global do Socket UDP para não ter que criar a cada vez
const udp_socket = UDPSocket()
const PORTA_ALVO = 9876
const IP_ALVO = ip"127.0.0.1"

"""
Envia as estatísticas de forma assíncrona (fogo-e-esquece).
Isso garante que o jogo não congele se o dashboard não estiver aberto.
"""
function enviar_estatisticas(geracao::Int, melhor_score::Float64, score_medio::Float64, melhor_jogo::Float64, medio_jogo::Float64)
    # A formatação do pacote: "geracao,melhor_score,score_medio,melhor_jogo,medio_jogo"
    mensagem = "$geracao,$melhor_score,$score_medio,$melhor_jogo,$medio_jogo"
    
    try
        send(udp_socket, IP_ALVO, PORTA_ALVO, mensagem)
    catch e
        # Falha silenciosa para não quebrar a execução do C++
        # (O UDP naturalmente não acusa erro se o receptor não estiver escutando, 
        # mas o catch previne erros locais da interface de rede)
    end
end

end
