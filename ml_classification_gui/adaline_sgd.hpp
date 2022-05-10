#include <algorithm>
#include <array>
#include <numeric>
#include <random>
#include <vector>

// adaptive linear neuron with stochastic gradient descent
template <size_t kFeatures>
class AdalineSGD {
   public:
    AdalineSGD(float eta = 0.01f, uint32_t random_state = 0);
    void initialize();
    void fit(const std::vector<std::array<float, kFeatures>>& x,
             const std::vector<int>& y, uint32_t n_iter = 1);
    void partialFit(const std::array<float, kFeatures>& x, int y);
    int predict(const std::array<float, kFeatures>& x) const;
    const std::vector<float>& getLosses() const { return cost_; }

   private:
    std::array<float, kFeatures + 1> w_;
    std::vector<float> cost_;
    float eta;
    std::mt19937 gen_;

    float updateWeights(const std::array<float, kFeatures>& x, int y);
    float netInput(const std::array<float, kFeatures>& x) const;
    float activation(float x) const { return x; }
};

template <size_t kFeatures>
AdalineSGD<kFeatures>::AdalineSGD(float eta, uint32_t random_state)
    : eta(eta), gen_(random_state) {
    initialize();
}

template <size_t kFeatures>
void AdalineSGD<kFeatures>::initialize() {
    cost_.clear();
    std::normal_distribution<float> nd(0.0, 0.01f);
    for (auto& i : w_) {
        i = nd(gen_);
    }
}

template <size_t kFeatures>
void AdalineSGD<kFeatures>::fit(
    const std::vector<std::array<float, kFeatures>>& x,
    const std::vector<int>& y, uint32_t n_iter) {
    std::vector<size_t> indexes(x.size());
    std::iota(indexes.begin(), indexes.end(), 0);
    for (uint32_t n = 0; n < n_iter; ++n) {
        std::shuffle(indexes.begin(), indexes.end(), gen_);
        float cost = 0;
        for (auto i : indexes) {
            cost += updateWeights(x[i], y[i]);
        }
        cost_.push_back(cost / static_cast<float>(x.size()));
    }
}

template <size_t kFeatures>
void AdalineSGD<kFeatures>::partialFit(const std::array<float, kFeatures>& x,
                                       int y) {
    cost_.push_back(updateWeights(x, y));
}

template <size_t kFeatures>
int AdalineSGD<kFeatures>::predict(
    const std::array<float, kFeatures>& x) const {
    return activation(netInput(x)) >= 0.0 ? 1 : -1;
}

template <size_t kFeatures>
float AdalineSGD<kFeatures>::updateWeights(
    const std::array<float, kFeatures>& x, int y) {
    float error = static_cast<float>(y) - activation(netInput(x));
    for (size_t j = 0; j != kFeatures; ++j) {
        w_[j] += eta * x[j] * error;
    }
    w_[kFeatures] += eta * error;
    return error * error * 0.5f;
}

template <size_t kFeatures>
float AdalineSGD<kFeatures>::netInput(
    const std::array<float, kFeatures>& x) const {
    float result = w_[kFeatures];
    for (size_t i = 0; i != kFeatures; ++i) {
        result += x[i] * w_[i];
    }
    return result;
}
