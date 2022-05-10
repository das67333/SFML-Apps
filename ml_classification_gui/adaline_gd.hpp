#include <array>
#include <random>
#include <vector>

// adaptive linear neuron with gradient descent
template <size_t kFeatures>
class AdalineGD {
   public:
    AdalineGD(float eta = 0.01f, uint32_t random_state = 0);
    void initialize();
    void fit(const std::vector<std::array<float, kFeatures>>& x,
             const std::vector<int>& y, uint32_t n_iter = 1);
    int predict(const std::array<float, kFeatures>& x) const;
    const std::vector<float>& getLosses() const { return cost_; }

   private:
    std::array<float, kFeatures + 1> w_;
    std::vector<float> cost_;
    float eta;
    std::mt19937 gen_;

    float updateWeights(const std::vector<std::array<float, kFeatures>>& x,
                        const std::vector<int>& y);
    float netInput(const std::array<float, kFeatures>& x) const;
    float activation(float x) const { return x; }
};

template <size_t kFeatures>
AdalineGD<kFeatures>::AdalineGD(float eta, uint32_t random_state)
    : eta(eta), gen_(random_state) {
    initialize();
}

template <size_t kFeatures>
void AdalineGD<kFeatures>::initialize() {
    cost_.clear();
    std::normal_distribution<float> nd(0.0, 0.01f);
    for (auto& i : w_) {
        i = nd(gen_);
    }
}

template <size_t kFeatures>
void AdalineGD<kFeatures>::fit(
    const std::vector<std::array<float, kFeatures>>& x,
    const std::vector<int>& y, uint32_t n_iter) {
    for (uint32_t n = 0; n < n_iter; ++n) {
        cost_.push_back(updateWeights(x, y) / static_cast<float>(x.size()));
    }
}

template <size_t kFeatures>
int AdalineGD<kFeatures>::predict(const std::array<float, kFeatures>& x) const {
    return activation(netInput(x)) >= 0.0 ? 1 : -1;
}

template <size_t kFeatures>
float AdalineGD<kFeatures>::updateWeights(
    const std::vector<std::array<float, kFeatures>>& x,
    const std::vector<int>& y) {
    float sum = 0, sum_sq = 0;
    std::array<float, kFeatures> delta = {0};
    for (size_t i = 0; i != x.size(); ++i) {
        float error = static_cast<float>(y[i]) - activation(netInput(x[i]));
        sum += error;
        sum_sq += error * error;
        for (size_t j = 0; j != kFeatures; ++j) {
            delta[j] += x[i][j] * error;
        }
    }
    for (size_t j = 0; j != kFeatures; ++j) {
        w_[j] += eta * delta[j];
    }
    w_[kFeatures] += eta * sum;
    return sum_sq * 0.5f;
}

template <size_t kFeatures>
float AdalineGD<kFeatures>::netInput(
    const std::array<float, kFeatures>& x) const {
    float result = w_[kFeatures];
    for (size_t i = 0; i != kFeatures; ++i) {
        result += x[i] * w_[i];
    }
    return result;
}
