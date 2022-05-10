#include <array>
#include <random>
#include <vector>

// Rosenblatt's perceptron
template <size_t kFeatures>
class Perceptron {
   public:
    Perceptron(float eta = 0.01f, uint32_t random_state = 0);
    void initialize();
    void fit(const std::vector<std::array<float, kFeatures>>& x,
             const std::vector<int>& y, uint32_t n_iter = 1);
    int predict(const std::array<float, kFeatures>& x) const;
    const std::vector<float>& getLosses() const { return errors_; }

   private:
    std::array<float, kFeatures + 1> w_;
    std::vector<float> errors_;
    float eta;
    std::mt19937 gen_;

    float netInput(const std::array<float, kFeatures>& x) const;
};

template <size_t kFeatures>
Perceptron<kFeatures>::Perceptron(float eta, uint32_t random_state)
    : eta(eta), gen_(random_state) {
    initialize();
}

template <size_t kFeatures>
void Perceptron<kFeatures>::initialize() {
    errors_.clear();
    std::normal_distribution<float> nd(0.0, 0.01f);
    for (auto& i : w_) {
        i = nd(gen_);
    }
}

template <size_t kFeatures>
void Perceptron<kFeatures>::fit(
    const std::vector<std::array<float, kFeatures>>& x,
    const std::vector<int>& y, uint32_t n_iter) {
    for (uint32_t n = 0; n < n_iter; ++n) {
        for (size_t i = 0; i != x.size(); ++i) {
            int delta = y[i] - predict(x[i]);
            errors_.push_back(static_cast<bool>(delta));
            float update = eta * static_cast<float>(delta);
            for (size_t j = 0; j != kFeatures; ++j) {
                w_[j] += update * x[i][j];
            }
            w_[kFeatures] += update;
        }
    }
}

template <size_t kFeatures>
int Perceptron<kFeatures>::predict(
    const std::array<float, kFeatures>& x) const {
    return netInput(x) >= 0.0 ? 1 : -1;
}

template <size_t kFeatures>
float Perceptron<kFeatures>::netInput(
    const std::array<float, kFeatures>& x) const {
    float result = w_[kFeatures];
    for (size_t i = 0; i != kFeatures; ++i) {
        result += x[i] * w_[i];
    }
    return result;
}
