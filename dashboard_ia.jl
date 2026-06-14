using Sockets
using Plots
using Statistics

function start_dashboard()
    porta = 9876
    receptor = UDPSocket()
    
    # Permite reaproveitar a porta se for fechada mal
    if bind(receptor, ip"127.0.0.1", porta)
        println("[DASHBOARD] Escutando telemetria na porta $porta...")
    else
        println("[DASHBOARD ERRO] Porta $porta já está em uso. Tente fechar outras instâncias.")
        return
    end

    # Arrays para guardar o historico apenas na Memória RAM
    geracoes = Int[]
    melhores = Float64[]
    medios = Float64[]
    melhores_jogo_array = Float64[]
    medios_jogo_array = Float64[]

    println("[DASHBOARD] Aguardando dados do jogo (Aquapura)...")
    
    # Gera a janela inicial vazia
    p = plot(title="Evolução da IA - Aquapura", xlabel="Geração", ylabel="Pontuação", legend=:topleft)
    display(p)

    try
        while true
            # Bloqueia e espera o próximo pacote (zero uso de CPU enquanto espera)
            pacote = recv(receptor)
            mensagem = String(pacote)
            
            partes = split(mensagem, ",")
            if length(partes) == 5
                try
                    ger = parse(Int, partes[1])
                    melhor_fit = parse(Float64, partes[2])
                    medio_fit = parse(Float64, partes[3])
                    melhor_jogo = parse(Float64, partes[4])
                    medio_jogo = parse(Float64, partes[5])
                    
                    push!(geracoes, ger)
                    push!(melhores, melhor_fit)
                    push!(medios, medio_fit)
                    push!(melhores_jogo_array, melhor_jogo)
                    push!(medios_jogo_array, medio_jogo)
                    
                    theme(:dark)
                    
                    bg_color = "#0D1117"
                    grid_color = "#30363D"
                    text_color = "#C9D1D9"
                    
                    janela_size = 150
                    inicio_janela = max(1, length(geracoes) - janela_size + 1)
                    
                    ger_view = geracoes[inicio_janela:end]
                    
                    # Gráfico 1: Fitness (Evolução da IA)
                    p1 = plot(ger_view, melhores[inicio_janela:end], 
                             label="  Melhor Fitness", color="#00F0FF", linewidth=3, fill=(0, 0.15, "#00F0FF"),
                             title="Treinamento: Fitness da IA", ylabel="Fitness Score",
                             grid=true, gridstyle=:dot, gridalpha=0.6, framestyle=:box, legend=:topleft)
                    plot!(p1, ger_view, medios[inicio_janela:end], label="  Média Fitness", color="#FF007F", linewidth=2.5, linestyle=:dash)
                    
                    max_fit = maximum(melhores)
                    hline!(p1, [max_fit], color="#39FF14", linestyle=:dot, linewidth=2, label="  Recorde: $(round(max_fit, digits=1))")
                    
                    # Gráfico 2: Jogo Real (Pontos Oficiais)
                    p2 = plot(ger_view, melhores_jogo_array[inicio_janela:end], 
                             label="  Melhor Score Jogo", color="#FFA500", linewidth=3, fill=(0, 0.15, "#FFA500"),
                             title="Realidade: Pontuação no Jogo (Lixo=1, Peixe<0)", xlabel="Geração", ylabel="Pontos Aquapura",
                             grid=true, gridstyle=:dot, gridalpha=0.6, framestyle=:box, legend=:topleft)
                    plot!(p2, ger_view, medios_jogo_array[inicio_janela:end], label="  Média Jogo", color="#8A2BE2", linewidth=2.5, linestyle=:dash)
                    
                    max_jogo = maximum(melhores_jogo_array)
                    hline!(p2, [max_jogo], color="#FFD700", linestyle=:dot, linewidth=2, label="  Recorde Jogo: $(round(max_jogo, digits=1))")
                    
                    # Combina os dois gráficos
                    p_final = plot(p1, p2, layout=(2, 1), 
                                  size=(900, 800), margin=6Plots.mm,
                                  background_color=bg_color, background_color_outside=bg_color,
                                  foreground_color_grid=grid_color, foreground_color_axis=grid_color,
                                  foreground_color_text=text_color, foreground_color_border=bg_color, fontfamily="sans-serif")
    
                    display(p_final)
                    println("=> Atualizado: Geração $ger | Fit Melhor: $(round(melhor_fit, digits=2)) | Jogo Melhor: $(round(melhor_jogo, digits=0))")
                catch e
                    println("[DASHBOARD] Pacote mal formatado ignorado: $mensagem")
                end
            end
        end
    catch e
        if isa(e, InterruptException)
            println("\n[DASHBOARD] Encerrando painel (CTRL+C detectado)...")
        else
            println("\n[DASHBOARD] Erro inesperado: ", e)
        end
    finally
        close(receptor)
        exit(0)
    end
end

start_dashboard()
