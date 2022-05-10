#include <random>

class Agent {
   public:
    Agent(uint32_t n_states, uint32_t n_actions, float learning_rate = 0.01f,
          float discount_factor = 0.9f, float epsilon = 0.9f,
          float epsilon_min = 0.1f, float epsilon_decay = 0.95f,
          uint32_t random_state = 0);
    uint32_t chooseAction(uint32_t state) const;
    void learn(uint32_t state);

   private:
    const uint32_t nStates, nActions;
    float learning_rate, discount_factor, epsilon, epsilon_min, epsilon_decay;
    std::vector<float> q_;
    mutable std::mt19937 gen_;
    mutable uint32_t state_prev, action_prev;
};

Agent::Agent(uint32_t n_states, uint32_t n_actions, float learning_rate,
             float discount_factor, float epsilon, float epsilon_min,
             float epsilon_decay, uint32_t random_state)
    : nStates(n_states),
      nActions(n_actions),
      learning_rate(learning_rate),
      discount_factor(discount_factor),
      epsilon(epsilon),
      epsilon_min(epsilon_min),
      epsilon_decay(epsilon_decay),
      q_(nActions * nStates),
      gen_(random_state) {}

uint32_t Agent::chooseAction(uint32_t state) const {
    uint32_t action;
    if (std::uniform_real_distribution(0.0, 1.0)(gen_) < epsilon) {
        action = std::uniform_int_distribution<uint32_t>(0, nActions - 1)(gen_);
    } else {
        auto it = q_.begin() + state * nActions;
        float max = *std::max(it, it + nActions);
        std::vector<uint32_t> actions_best;
        for (uint32_t i = 0; i != nActions; ++i) {
            if (q_[i] >= max) {
                actions_best.push_back(i);
            }
        }
        action = actions_best[std::uniform_int_distribution<uint32_t>(
            0, static_cast<uint32_t>(actions_best.size()) - 1)(gen_)];
    }
    state_prev = state;
    action_prev = action;
    return action;
}

void Agent::learn(uint32_t state) {
    auto it = q_.begin() + state_prev * nActions;
    int r = 0;
    q_[action_prev + state_prev * nActions] +=
        learning_rate *
        (r + discount_factor *
                 *std::max(it, it + nActions -
                                   q_[action_prev + state_prev * nActions]));
}
