#include <algorithm>
#include <bitset>
#include <condition_variable>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <cassert>
#include <maze_utils.hpp>
#include <nn.hpp>
#include <researcher.hpp>

using namespace utils;

std::string back_string() {
    static std::string s;
    if (s.empty()) {
        s = std::string(100, '\b');
    }
    return s;
}

template <size_t N, size_t K>
auto createResearchers() {
    return ([]<size_t... index>(std::index_sequence<index...>) {
        return std::tuple_cat(
            (index, (std::conditional_t<(3 + index % 8) * (3 + index / 8) <= K,
                                        std::tuple<Researcher<(5 + index % 8), (5 + index / 8)>>, std::tuple<>>{}))...);
    })(std::make_index_sequence<N>{});
}

template <typename TupleT, typename Fn>
void for_each_tuple(TupleT &&tp, Fn &&fn) {
    std::apply([&fn]<typename... T>(T &&...args) { (fn(std::forward<T>(args)), ...); }, std::forward<TupleT>(tp));
}

template <size_t M, size_t N>
bool test_fast_bruteforce(size_t max_threads = 8) {
    Researcher<M, N> r;
    bool result = true;
    for (size_t i = 1; i < max_threads; i++) {
        result &= r.check_threaded_find_best_bruteforce(i);
        if (result == false) {
            std::cout << "Test failed for " << M << "x" << N << " with " << i << " threads" << std::endl;
            break;
        } else {
            // std::cout << "Test passed for " << M << "x" << N << " with " << i << " threads" << std::endl;
        }
    }
    return result;
}

// задумка: создать датасет из лабиринта, проходим по лабиринту, берём квадрат 9x9 вокруг квадрата 3x3. 9x9 за
// вычетом 3x3 это вход, 3x3 это выход
template <size_t M, size_t N>
std::vector<std::pair<std::bitset<72>, std::bitset<9>>> get_dataset_from_maze(maze<M, N> m) {
    std::vector<std::pair<std::bitset<72>, std::bitset<9>>> dataset;

    for (int i = 2; i < M - 2; i++) {
        for (int j = 2; j < N - 2; j++) {
            std::bitset<72> input;
            std::bitset<9> output;

            int index_input = 0;
            int index_output = 0;

            for (int x = i - 4; x < i + 5; x++) {
                for (int y = j - 4; y < j + 5; y++) {

                    // пропустим 3x3 в центре
                    if (x >= i - 1 && x <= i + 1 && y >= j - 1 && y <= j + 1) {
                        // std::cout << "skip " << x << " " << y << std::endl;
                        continue;
                    }

                    if (x < 0 || x > M - 1 || y < 0 || y > N - 1) {
                        input[index_input] = 1;
                    } else {
                        input[index_input] = m[x][y] < MX ? 0 : 1;
                    }

                    index_input++;
                }
            }

            for (int x = i - 1; x < i + 2; x++) {
                for (int y = j - 1; y < j + 2; y++) {
                    output[index_output] = m[x][y] < MX ? 0 : 1;
                    index_output++;
                }
            }

            // проверим, что index_input и index_output равны 72 и 9
            if (index_input != 72 || index_output != 9) {
                std::cerr << "Error: index_input = " << index_input << ", index_output = " << index_output << std::endl;
                return dataset;
            }

            dataset.push_back(std::make_pair(input, output));
        }
    }

    return dataset;
}

using Dataset = std::vector<std::pair<std::bitset<72>, std::bitset<9>>>;

template <size_t K> // 2**K is max combinations
Dataset small_researchers() {
    auto researchers = createResearchers<128, K>();

    std::cout << "Researchers count: " << std::tuple_size<decltype(researchers)>::value << std::endl;

    std::vector<std::thread> threads; // workers

    for_each_tuple(researchers, [&](auto &i) {
        // auto [m, n] = i.get_dimensions();
        // auto combinations = i.get_combinations();
        // std::cout << "Maze " << m << "x" << n << " combinations: " << combinations << std::endl;
        threads.push_back(std::thread([&i]() { i.threaded_find_best_bruteforce(); }));
    });

    for (auto &t : threads) {
        t.join();
    }

    for_each_tuple(researchers, [](auto &i) { i.extended_show_maze(); });

    Dataset dataset;
    std::mutex mtx;

    for_each_tuple(researchers, [&](auto &i) {
        auto d = get_dataset_from_maze<i.m_, i.n_>(i.m);
        std::lock_guard<std::mutex> lock(mtx);
        dataset.insert(dataset.end(), d.begin(), d.end());
    });

    return dataset;
}

#define D std::cout << __FILE__ << ":" << __LINE__ << std::endl;

Dataset get_dataset_from_search(size_t size) {
    Dataset dataset;
    maze<21, 31> m;
    prepare_maze<21, 31>(m);
    while (dataset.size() < size) {
        // для начала немного апдейтнем лабиринт
        MutationManager<21, 31> mm;

        size_t score = pass_maze<21, 31>(m);
        clean_maze<21, 31>(m);
        for (int i = 0; i < 10; i++) {
            // std::cout << "Iteration " << i << " with score " << score << std::endl;
            mm.apply_random_mutation(m);
            if (!is_solvable<21, 31>(m)) {
                mm.deny_last_mutation(m);
                i--;
                continue;
            }
            size_t new_score = pass_maze<21, 31>(m);
            clean_maze<21, 31>(m);
            if (new_score > score * 0.99 - 10) {
                score = new_score;
                // system("cls");
                // std::cout << "Score: " << score << std::endl;
                // show_maze<21, 31>(m);
            } else {
                mm.deny_last_mutation(m);
                i--;
            }
        }

        // теперь выберем случайную точку в лабиринте
        int x = (r() % (18 - 2 + 1)) + 2;
        int y = (r() % (28 - 2 + 1)) + 2;

        assert(x >= 2 && x <= 18);
        assert(y >= 2 && y <= 28);

        // переберём все возможные варианты в квадрате 3x3 вокруг точки

        // для начала обнулим весь квадрат
        // for (int i = x - 1; i < x + 2; i++) {
        //     for (int j = y - 1; j < y + 2; j++) {
        //         m[i][j] = 0;
        //     }
        // }

        // функция итерации квадрата
        auto iterate_square = [&]() {
            for (int i = x - 1; i < x + 2; i++) {
                for (int j = y - 1; j < y + 2; j++) {
                    if (m[i][j] < MX) {
                        m[i][j] = MX;
                        return;
                    }
                    m[i][j] = 0;
                }
            }
        };

        // функция проверки, что квадрат пуст
        auto is_square_empty = [&]() {
            for (int i = x - 1; i < x + 2; i++) {
                for (int j = y - 1; j < y + 2; j++) {
                    if (m[i][j] >= MX) {
                        return false;
                    }
                }
            }
            return true;
        };

        // теперь сам цикл
        size_t best_score = 0;
        std::bitset<9> best_m; // лучший квадрат
        for (int c = 0; c < 512; c++) {

            if (!is_solvable<21, 31>(m)) {
                iterate_square();
                continue;
            }

            size_t current_score = pass_maze<21, 31>(m);
            clean_maze<21, 31>(m);

            if (current_score > best_score) {
                best_score = current_score;
                int b_index = 0;
                for (int i = x - 1; i < x + 2; i++) {
                    for (int j = y - 1; j < y + 2; j++) {
                        best_m[b_index++] = m[i][j] < MX ? 0 : 1;
                    }
                }
            }

            iterate_square();
        };

        // while (!is_solvable<21, 31>(m)) {
        //     std::cout << "Not solvable" << std::endl;
        //     iterate_square();
        // }

        if (best_score <= score) {
            int b_index = 0;
            for (int i = x - 1; i < x + 2; i++) {
                for (int j = y - 1; j < y + 2; j++) {
                    m[i][j] = best_m[b_index++] ? MX : 0;
                }
            }
            continue;
        }

        // if (dataset.size() % 10 == 0) {
        //     std::cout << back_string();
        //     std::cout << "Best score: " << best_score << ", current score: " << score
        //               << ", dataset size: " << dataset.size() << "         ";
        // }

        std::bitset<72> input;

        int index_input = 0;

        for (int i = x - 4; i < x + 5; i++) {
            for (int j = y - 4; j < y + 5; j++) {

                // пропустим 3x3 в центре
                if (i >= x - 1 && i <= x + 1 && j >= y - 1 && j <= y + 1) {
                    // std::cout << "skip " << x << " " << y << std::endl;
                    continue;
                }

                if (i < 0 || i > 20 || j < 0 || j > 30) {
                    input[index_input] = 1;
                } else {
                    input[index_input] = m[i][j] < MX ? 0 : 1;
                }

                index_input++;
            }
        }

        // проверим, что index_input равен 72
        if (index_input != 72) {
            std::cerr << "Error: index_input = " << index_input << std::endl;
            return dataset;
        }

        dataset.push_back(std::make_pair(input, best_m));

        // не забудем заменить квадрат на лучший
        // int b_index = 0;
        // for (int i = x - 1; i < x + 2; i++) {
        //     for (int j = y - 1; j < y + 2; j++) {
        //         m[i][j] = best_m[b_index++] ? MX : 0;
        //     }
        // }
    }
    // std::cout << std::endl;
    //  std::cout << "Done with dataset at score " << pass_maze<21, 31>(m) << std::endl;

    return dataset;
}

/*
void strange_researcher() {
    Researcher<21, 31> r;

    r.show_maze();
}
*/

void try_small_dataset() {
    // remove saves folder
    // std::filesystem::remove_all("./saves");

    auto dataset = small_researchers<36>();

    // Researcher<5, 6> r;

    // r.threaded_find_best_bruteforce();
    // r.extended_show_maze();

    // auto dataset = get_dataset_from_maze<5, 6>(r.m);

    std::cout << "Dataset size: " << dataset.size() << std::endl;

    // show dataset
    // for (const auto &d : dataset) {
    //     std::cout << d.first << " -> " << d.second << std::endl;
    // }

    // create neural network
    NeuralNetwork<72, 7, 72, 9, ActivationFunction::Sigmoid> nn;

    // train neural network
    nn.Train(dataset, 0.3, 10000, 2e-4, 0.01);

    // test neural network
    std::cout << "Score: " << nn.Score(dataset) << std::endl;
}

Dataset get_large_dataset(size_t batch_size, size_t batch_number) {
    std::vector<std::thread> threads;
    std::vector<Dataset> datasets;

    for (size_t i = 0; i < batch_number; i++) {
        threads.push_back(std::thread([i, batch_size, &datasets]() {
            datasets.push_back(get_dataset_from_search(batch_size));
            std::cout << "Batch " << i << " done" << std::endl;
        }));
    }

    for (auto &t : threads) {
        t.join();
    }

    Dataset dataset;

    for (const auto &d : datasets) {
        dataset.insert(dataset.end(), d.begin(), d.end());
    }

    // remove non unique elements
    std::sort(dataset.begin(), dataset.end(),
              [](const auto &a, const auto &b) { return a.first.to_string() < b.first.to_string(); });
    dataset.erase(std::unique(dataset.begin(), dataset.end(),
                              [](const auto &a, const auto &b) { return a.first.to_string() == b.first.to_string(); }),
                  dataset.end());

    // shuffle dataset
    std::random_shuffle(dataset.begin(), dataset.end());

    return dataset;
}

bool write_dataset_to_file(const Dataset &dataset, const std::string &filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    for (const auto &d : dataset) {
        file.write(reinterpret_cast<const char *>(&d.first), sizeof(d.first));
        file.write(reinterpret_cast<const char *>(&d.second), sizeof(d.second));
    }

    file.close();

    return true;
}

Dataset read_dataset_from_file(const std::string &filename) {
    Dataset dataset;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return dataset;
    }

    while (true) {
        std::bitset<72> input;
        std::bitset<9> output;

        file.read(reinterpret_cast<char *>(&input), sizeof(input));
        file.read(reinterpret_cast<char *>(&output), sizeof(output));

        if (file.eof()) {
            break;
        }

        dataset.push_back(std::make_pair(input, output));
    }

    file.close();

    return dataset;
}

bool compate_datasets(const Dataset &d1, const Dataset &d2) {
    if (d1.size() != d2.size()) {
        return false;
    }

    for (size_t i = 0; i < d1.size(); i++) {
        if (d1[i].first != d2[i].first || d1[i].second != d2[i].second) {
            return false;
        }
    }

    return true;
}

bool test_dataset_io() {
    Dataset dataset = get_large_dataset(150, 30);

    std::string filename = "dataset.bin";

    if (!write_dataset_to_file(dataset, filename)) {
        return false;
    }

    Dataset dataset2 = read_dataset_from_file(filename);

    return compate_datasets(dataset, dataset2);
}

void sample_nn() {

    auto dataset = get_large_dataset(150, 30);

    // Dataset dataset = f();
    std::cout << "Dataset size: " << dataset.size() << std::endl;

    // for (const auto &d : dataset) {
    //     std::cout << d.first << " -> " << d.second << std::endl;
    // }

    // create neural network
    NeuralNetwork<72, 7, 81, 9, ActivationFunction::Sigmoid> nn;

    // train neural network
    nn.Train(dataset, 0.2, 20000, 3e-4, 0.001);

    // test neural network
    std::cout << "Score: " << nn.Score(dataset) << std::endl;

    // std::cout << "saved to file: " << nn.save_to_file("nn.txt") << std::endl;

    // load neural network
    // NeuralNetwork<72, 7, 81, 9> nn2;
    // std::cout << "loaded from file: " << nn2.load_from_file("nn.txt") << std::endl;

    // test neural network
    // std::cout << "Score: " << nn2.Score(dataset) << std::endl;
}

/*
class Trainer {
  public:
    std::vector<NeuralNetwork<72, 7, 81, 9, ActivationFunction::Sigmoid>> nn;
    Dataset dataset;
    std::vector<std::thread> threads;

    Trainer(int num_workers, Dataset dataset) : nn(num_workers) {
        this->dataset = dataset;

        for (size_t i = 0; i < num_workers; i++) {
            threads.push_back(
                std::thread([this, i]() { this->nn[i].Train(this->dataset, 0.5 + i / 10., 5000, 3e-4, 0.0001); }));
        }
    }

    void join() {
        for (auto &t : threads) {
            t.join();
        }
        threads.clear();
    }

    void save_to_file(const std::string &filename) {
        for (size_t i = 0; i < nn.size(); i++) {
            nn[i].save_to_file(filename + "_" + std::to_string(i) + ".nn");
        }
    }

    void print_scores() {
        for (size_t i = 0; i < nn.size(); i++) {
            std::cout << "Score " << i << ": " << nn[i].Score(dataset) << std::endl;
        }
    }
};

class SelectiveTrainer {
  public:
    using NN = NeuralNetwork<72, 7, 81, 9, ActivationFunction::Sigmoid>;

    Dataset dataset;
    NN nn;

    SelectiveTrainer(Dataset dataset) { this->dataset = dataset; }

    void iteration(int num_workers, int num_iterations) {
        // shuffle dataset
        std::random_shuffle(dataset.begin(), dataset.end());

        std::vector<NN> nns(num_workers);
        for (auto &n : nns) {
            n = nn;
        }

        std::vector<std::thread> threads;
        std::vector<double> last_errors;
        // D
        for (size_t i = 0; i < num_workers; i++) {
            last_errors.push_back(0);
            threads.push_back(std::thread([this, i, num_iterations, &nns, &last_errors]() {
                last_errors[i] = nns[i].Train(this->dataset, 0.000001 + i / 1000000., num_iterations, 3e-4, 0.0001);
            }));
        }
        // D
        for (auto &t : threads) {
            t.join();
        }
        // D
        double best_error = std::numeric_limits<double>::max();
        size_t best_index = 0;

        for (size_t i = 0; i < num_workers; i++) {
            if (last_errors[i] < best_error) {
                best_error = last_errors[i];
                best_index = i;
            }
        }

        nn = nns[best_index];

        std::cout << "Best error: " << best_error << std::endl;
        std::cout << "Best score: " << nn.Score(dataset) << std::endl;
    }

    NN get_nn() { return nn; }
};

void selective_trainer() {
    Dataset dataset = read_dataset_from_file("good_100k_2.bin");

    SelectiveTrainer st(dataset);

    for (int i = 0; i < 100; i++) {
        st.iteration(10, 10);
    }

    auto nn = st.get_nn();

    nn.save_to_file("good_nn_100k_2.nn");
}

*/