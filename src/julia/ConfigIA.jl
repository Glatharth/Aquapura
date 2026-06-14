module ConfigIA

# Concurrent AI agents simulated per training session
const QTD_PLAYERS = 10

# Step threshold for PPO rollout buffer processing
const ROLLOUT_STEPS = 256

# Maximum idle frames allowed before episode termination (suicide penalty)
# 60 frames = 1 second. Example: 60 * 3 = 3 seconds.
const LIMITE_INATIVIDADE = 60 * 3

# Optimizer learning rate
const LEARNING_RATE = 1e-3

# Exploration factor (Entropy coefficient) - Higher means more random actions to discover new strategies
const ENTROPY_COEF = 0.02

# Discount factor (Gamma) for future rewards calculation
const GAMMA = 0.99

# Minibatch size for GPU/CPU gradient updates
const BATCH_SIZE = 1024

# Number of optimization epochs per rollout buffer
const EPOCHS = 4

# Serialization path for neural network weights
const ARQUIVO_PESOS = "pesos_ia.bson"

# Neural network input tensor dimension (sensor count)
const NUM_ENTRADAS = 30

end
