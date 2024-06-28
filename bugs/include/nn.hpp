#include <array>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

// #include <omp.h>

enum class ActivationFunction { Sigmoid, LeakyReLU };

template <std::size_t InputNeurons, std::size_t HiddenLayers, std::size_t HiddenNeurons, std::size_t OutputNeurons,
          ActivationFunction AF>
class NeuralNetwork {
  public:
    NeuralNetwork() : gen(rd()), dist(-1, 1) { InitializeWeights(); }

    NeuralNetwork(const NeuralNetwork<InputNeurons, HiddenLayers, HiddenNeurons, OutputNeurons, AF> &other)
        : gen(rd()), dist(-1, 1) {
        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                weights_input_hidden_[i][j] = other.weights_input_hidden_[i][j];
            }
        }

        for (std::size_t h = 0; h < HiddenLayers - 1; ++h) {
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    weights_hidden_hidden_[h][i][j] = other.weights_hidden_hidden_[h][i][j];
                }
            }
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                weights_hidden_output_[i][j] = other.weights_hidden_output_[i][j];
            }
        }

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            bias_hidden_[i] = other.bias_hidden_[i];
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            bias_output_[i] = other.bias_output_[i];
        }
    }

    void operator=(const NeuralNetwork<InputNeurons, HiddenLayers, HiddenNeurons, OutputNeurons, AF> &other) {
        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                weights_input_hidden_[i][j] = other.weights_input_hidden_[i][j];
            }
        }

        for (std::size_t h = 0; h < HiddenLayers - 1; ++h) {
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    weights_hidden_hidden_[h][i][j] = other.weights_hidden_hidden_[h][i][j];
                }
            }
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                weights_hidden_output_[i][j] = other.weights_hidden_output_[i][j];
            }
        }

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            bias_hidden_[i] = other.bias_hidden_[i];
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            bias_output_[i] = other.bias_output_[i];
        }
    }

    double Train(const std::vector<std::pair<std::bitset<InputNeurons>, std::bitset<OutputNeurons>>> &dataset,
               double learning_rate, std::size_t epochs, double decay_rate, double l2_lambda) {
                double last_error = std::numeric_limits<double>::max();
        for (std::size_t epoch = 0; epoch < epochs; ++epoch) {
            double total_error = 0.0;

            double current_learning_rate = learning_rate * std::exp(-decay_rate * epoch);

            for (const auto &[input, target] : dataset) {
                total_error += TrainIteration(input, target, current_learning_rate, l2_lambda);
            }
            last_error = total_error;

            bool print = (epoch < 10) || (epoch < 100 && epoch % 10 == 0) || (epoch < 1000 && epoch % 100 == 0) ||
                         (epoch % 1000 == 0);

            if (print) {
                std::cout << "Epoch " << epoch << ": error = " << total_error
                          << ", learning rate = " << current_learning_rate << std::endl;
            }

            if (total_error < 1e-3) {
                std::cout << "Converged at epoch " << epoch << std::endl;
                break;
            }
        }
        return last_error;
    }

    std::bitset<OutputNeurons> Apply(const std::bitset<InputNeurons> &input) const {
        std::array<double, HiddenNeurons> hidden_input{};
        std::array<std::array<double, HiddenNeurons>, HiddenLayers> hidden_outputs{};
        std::array<double, OutputNeurons> output_input{};
        std::bitset<OutputNeurons> output;

        // Input layer -> first hidden layer
        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                hidden_input[i] += input[j] * weights_input_hidden_[i][j];
            }
            hidden_input[i] += bias_hidden_[i];
            if constexpr (AF == ActivationFunction::Sigmoid) {
                hidden_outputs[0][i] = Sigmoid(hidden_input[i]);
            } else {
                hidden_outputs[0][i] = LeakyReLU(hidden_input[i]);
            }
        }

        // Hidden layers
        for (std::size_t h = 1; h < HiddenLayers; ++h) {
            std::fill(hidden_input.begin(), hidden_input.end(), 0.0);
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    hidden_input[i] += hidden_outputs[h - 1][j] * weights_hidden_hidden_[h - 1][i][j];
                }
                hidden_input[i] += bias_hidden_[i];
                if constexpr (AF == ActivationFunction::Sigmoid) {
                    hidden_outputs[h][i] = Sigmoid(hidden_input[i]);
                } else {
                    hidden_outputs[h][i] = LeakyReLU(hidden_input[i]);
                }
            }
        }

        // Last hidden layer -> output layer
        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                output_input[i] += hidden_outputs[HiddenLayers - 1][j] * weights_hidden_output_[i][j];
            }
            output_input[i] += bias_output_[i];
            output[i] = (Sigmoid(output_input[i]) > 0.5);
        }

        return output;
    }

    double Score(const std::vector<std::pair<std::bitset<InputNeurons>, std::bitset<OutputNeurons>>> &dataset) const {
        double correct = 0.0;
        for (const auto &[input, target] : dataset) {
            std::bitset<OutputNeurons> output = Apply(input);
            if (output == target) {
                ++correct;
            }
        }
        return correct / dataset.size();
    }

  public:
    static double Sigmoid(double x) { return 1.0 / (1.0 + std::exp(-x)); }

    static double SigmoidDerivative(double x) { return x * (1.0 - x); }

    static double LeakyReLU(double x) { return x > 0 ? x : 0.01 * x; }

    static double LeakyReLUDerivative(double x) { return x > 0 ? 1 : 0.01; }

    void InitializeWeights() {

        // std::uniform_real_distribution<double> dist_input_hidden(-std::sqrt(6.0 / (InputNeurons + HiddenNeurons)),
        //                                                          std::sqrt(6.0 / (InputNeurons + HiddenNeurons)));

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                weights_input_hidden_[i][j] = dist(gen);
            }
        }
        // std::uniform_real_distribution<double> dist_hidden_hidden(-std::sqrt(6.0 / (HiddenNeurons + HiddenNeurons)),
        //                                                           std::sqrt(6.0 / (HiddenNeurons + HiddenNeurons)));
        for (std::size_t h = 0; h < HiddenLayers - 1; ++h) {
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    weights_hidden_hidden_[h][i][j] = dist(gen);
                }
            }
        }

        // std::uniform_real_distribution<double> dist_hidden_output(-std::sqrt(6.0 / (HiddenNeurons + OutputNeurons)),
        //                                                           std::sqrt(6.0 / (HiddenNeurons + OutputNeurons)));

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                weights_hidden_output_[i][j] = dist(gen);
            }
        }

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            bias_hidden_[i] = 0;
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            bias_output_[i] = 0;
        }
    }

    double TrainIteration(const std::bitset<InputNeurons> &input, const std::bitset<OutputNeurons> &target,
                          double learning_rate, double l2_lambda) {
        std::array<double, HiddenNeurons> hidden_input{};
        std::array<std::array<double, HiddenNeurons>, HiddenLayers> hidden_outputs{};
        std::array<double, OutputNeurons> output_input{};
        std::array<double, OutputNeurons> output{};

        // Forward pass
        // Input layer -> first hidden layer
        // #pragma omp parallel for num_threads(8)
        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            hidden_input[i] = bias_hidden_[i];
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                hidden_input[i] += input[j] * weights_input_hidden_[i][j];
            }
            if constexpr (AF == ActivationFunction::Sigmoid) {
                hidden_outputs[0][i] = Sigmoid(hidden_input[i]);
            } else {
                hidden_outputs[0][i] = LeakyReLU(hidden_input[i]);
            }
        }

        // Hidden layers
        for (std::size_t h = 1; h < HiddenLayers; ++h) {
            std::fill(hidden_input.begin(), hidden_input.end(), 0.0);
            // #pragma omp parallel for num_threads(8)
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                hidden_input[i] = bias_hidden_[i];
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    hidden_input[i] += hidden_outputs[h - 1][j] * weights_hidden_hidden_[h - 1][i][j];
                }
                if constexpr (AF == ActivationFunction::Sigmoid) {
                    hidden_outputs[h][i] = Sigmoid(hidden_input[i]);
                } else {
                    hidden_outputs[h][i] = LeakyReLU(hidden_input[i]);
                }
            }
        }

        // Last hidden layer -> output layer
        // #pragma omp parallel for num_threads(8)
        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            output_input[i] = bias_output_[i];
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                output_input[i] += hidden_outputs[HiddenLayers - 1][j] * weights_hidden_output_[i][j];
            }
            output[i] = Sigmoid(output_input[i]);
        }

        // Backward pass
        std::array<double, OutputNeurons> output_error{};
        std::array<std::array<double, HiddenNeurons>, HiddenLayers> hidden_error{};

        // Output layer error
        // #pragma omp parallel for num_threads(8)
        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            output_error[i] = (double(target[i]) - output[i]) * SigmoidDerivative(output[i]);
        }

        // Hidden layers error
        // #pragma omp parallel for num_threads(8)
        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            hidden_error[HiddenLayers - 1][i] = 0.0;
            for (std::size_t j = 0; j < OutputNeurons; ++j) {
                hidden_error[HiddenLayers - 1][i] += output_error[j] * weights_hidden_output_[j][i];
            }
            if constexpr (AF == ActivationFunction::Sigmoid) {
                hidden_error[HiddenLayers - 1][i] *= SigmoidDerivative(hidden_outputs[HiddenLayers - 1][i]);
            } else {
                hidden_error[HiddenLayers - 1][i] *= LeakyReLUDerivative(hidden_outputs[HiddenLayers - 1][i]);
            }
        }

        for (int h = int(HiddenLayers) - 2; h >= 0; --h) {
            // #pragma omp parallel for num_threads(8)
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                hidden_error[h][i] = 0.0;
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    hidden_error[h][i] += hidden_error[h + 1][j] * weights_hidden_hidden_[h][j][i];
                }
                if constexpr (AF == ActivationFunction::Sigmoid) {
                    hidden_error[h][i] *= SigmoidDerivative(hidden_outputs[h][i]);
                } else {
                    hidden_error[h][i] *= LeakyReLUDerivative(hidden_outputs[h][i]);
                }
            }
        }

        // Update weights and biases
        // #pragma omp parallel for num_threads(8)
        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                weights_hidden_output_[i][j] += learning_rate * (output_error[i] * hidden_outputs[HiddenLayers - 1][j] -
                                                                 l2_lambda * weights_hidden_output_[i][j]);
            }
            bias_output_[i] += learning_rate * output_error[i];
        }

        for (std::size_t h = 0; h < HiddenLayers; ++h) {
            // #pragma omp parallel for num_threads(8)
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                if (h == 0) {
                    for (std::size_t j = 0; j < InputNeurons; ++j) {
                        weights_input_hidden_[i][j] +=
                            learning_rate * (hidden_error[h][i] * input[j] - l2_lambda * weights_input_hidden_[i][j]);
                    }
                } else {
                    for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                        weights_hidden_hidden_[h - 1][i][j] +=
                            learning_rate * (hidden_error[h][i] * hidden_outputs[h - 1][j] -
                                             l2_lambda * weights_hidden_hidden_[h - 1][i][j]);
                    }
                }
                bias_hidden_[i] += learning_rate * hidden_error[h][i];
            }
        }

        // Calculate error
        double total_error = 0.0;
        // #pragma omp parallel for reduction(+:total_error) num_threads(8)
        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            total_error += std::pow(double(target[i]) - output[i], 2);
        }

        return total_error;
    }

    bool save_to_file(std::string filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << InputNeurons << " " << HiddenLayers << " " << HiddenNeurons << " " << OutputNeurons << std::endl;

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                file << weights_input_hidden_[i][j] << " ";
            }
            file << std::endl;
        }

        for (std::size_t h = 0; h < HiddenLayers - 1; ++h) {
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    file << weights_hidden_hidden_[h][i][j] << " ";
                }
                file << std::endl;
            }
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                file << weights_hidden_output_[i][j] << " ";
            }
            file << std::endl;
        }

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            file << bias_hidden_[i] << " ";
        }
        file << std::endl;

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            file << bias_output_[i] << " ";
        }
        file << std::endl;

        file.close();
        return true;
    }

    bool load_from_file(std::string filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::size_t input_neurons, hidden_layers, hidden_neurons, output_neurons;
        file >> input_neurons >> hidden_layers >> hidden_neurons >> output_neurons;

        if (input_neurons != InputNeurons || hidden_layers != HiddenLayers || hidden_neurons != HiddenNeurons ||
            output_neurons != OutputNeurons) {
            return false;
        }

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            for (std::size_t j = 0; j < InputNeurons; ++j) {
                file >> weights_input_hidden_[i][j];
            }
        }

        for (std::size_t h = 0; h < HiddenLayers - 1; ++h) {
            for (std::size_t i = 0; i < HiddenNeurons; ++i) {
                for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                    file >> weights_hidden_hidden_[h][i][j];
                }
            }
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            for (std::size_t j = 0; j < HiddenNeurons; ++j) {
                file >> weights_hidden_output_[i][j];
            }
        }

        for (std::size_t i = 0; i < HiddenNeurons; ++i) {
            file >> bias_hidden_[i];
        }

        for (std::size_t i = 0; i < OutputNeurons; ++i) {
            file >> bias_output_[i];
        }

        file.close();
        return true;
    }

    std::array<std::array<double, InputNeurons>, HiddenNeurons> weights_input_hidden_;
    std::array<std::array<std::array<double, HiddenNeurons>, HiddenNeurons>, HiddenLayers - 1> weights_hidden_hidden_;
    std::array<std::array<double, HiddenNeurons>, OutputNeurons> weights_hidden_output_;
    std::array<double, HiddenNeurons> bias_hidden_;
    std::array<double, OutputNeurons> bias_output_;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
};
