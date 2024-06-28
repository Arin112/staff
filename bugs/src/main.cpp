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

#define D std::cout << __FILE__ << ":" << __LINE__ << std::endl;

std::string back_string() {
    static std::string s;
    if (s.empty()) {
        s = std::string(100, '\b');
    }
    return s;
}

template <size_t size>
std::string to_string(const std::bitset<size> &b) {
    return b.to_string();
}

template <size_t size>
std::bitset<size> from_string(const std::string &s) {
    return std::bitset<size>(s);
}

template <size_t size>
std::string to_csv(const std::bitset<size> &b) {
    std::string s;
    for (size_t i = 0; i < size; i++) {
        s += b[i] ? "1" : "0";
        if (i < size - 1) {
            s += ",";
        }
    }
    return s;
}

// compact float to string
std::string to_string(float f) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << f;
    // strip trailing zeros
    std::string s = ss.str();
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    // strip trailing point
    if (s.back() == '.') {
        s.pop_back();
    }
    return s;
}

template <typename T>
std::string to_string(const std::vector<T> &v) {
    std::string s;
    for (const auto &e : v) {
        s += to_string(e) + " ";
    }
    return s;
}

template <typename T>
T from_string(const std::string &s) {
    T v;
    std::istringstream iss(s);
    std::remove_reference_t<decltype(v[0])> e;
    while (iss >> e) {
        v.push_back(e);
    }
    return v;
}

template <typename T>
std::string to_csv(const std::vector<T> &v) {
    std::string s;
    for (size_t i = 0; i < v.size(); i++) {
        s += std::to_string(v[i]);
        if (i < v.size() - 1) {
            s += ",";
        }
    }
    return s;
}

template <typename T_IN, typename T_OUT>
class Dataset {
  public:
    std::vector<std::pair<T_IN, T_OUT>> data;
    Dataset() = default;
    Dataset(const std::vector<std::pair<T_IN, T_OUT>> &data) : data(data) {}
    Dataset(const Dataset<T_IN, T_OUT> &d) : data(d.data) {}
    Dataset<T_IN, T_OUT> &operator=(const Dataset<T_IN, T_OUT> &d) {
        data = d.data;
        return *this;
    }
    void add(const T_IN &input, const T_OUT &output) { data.push_back(std::make_pair(input, output)); }
    void add(const std::pair<T_IN, T_OUT> &p) { data.push_back(p); }
    void clear() { data.clear(); }
    size_t size() const { return data.size(); }
    std::pair<T_IN, T_OUT> &operator[](size_t index) { return data[index]; }
    const std::pair<T_IN, T_OUT> &operator[](size_t index) const { return data[index]; }
    void shuffle() { std::shuffle(data.begin(), data.end(), std::mt19937(std::random_device()())); }
    void unique() {
        std::sort(data.begin(), data.end(),
                  [](const auto &a, const auto &b) { return to_string(a.first) < to_string(b.first); });
        data.erase(std::unique(data.begin(), data.end(),
                               [](const auto &a, const auto &b) { return to_string(a.first) == to_string(b.first); }),
                   data.end());
        shuffle();
    }
    bool write_to_file(const std::string &filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        file << sizeof(T_IN) << " " << sizeof(T_OUT) << " " << data.size() << std::endl;

        for (const auto &d : data) {
            file << to_string(d.first) << std::endl;
            file << to_string(d.second) << std::endl;
        }

        file.close();
        return true;
    }

    bool read_from_file(const std::string &filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        size_t size;
        size_t size_in, size_out;

        file >> size_in >> size_out >> size;

        if (size_in != sizeof(T_IN) || size_out != sizeof(T_OUT)) {
            return false;
        }

        data.reserve(data.size() + size);

        for (size_t i = 0; i < size; i++) {
            std::string in;
            std::string out;

            std::getline(file, in);
            std::getline(file, in);

            T_IN input = from_string<T_IN>(in);
            T_OUT output = from_string<T_OUT>(out);

            data.push_back(std::make_pair(input, output));
        }

        file.close();

        return true;
    }

    bool operator==(const Dataset<T_IN, T_OUT> &d) const {
        if (data.size() != d.data.size()) {
            return false;
        }

        for (size_t i = 0; i < data.size(); i++) {
            if (data[i].first != d.data[i].first || data[i].second != d.data[i].second) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const Dataset<T_IN, T_OUT> &d) const { return !(*this == d); }

    bool save_as_csv(const std::string &filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        for (auto &d : data) {
            file << to_csv(d.first) << "," << to_csv(d.second) << std::endl;
        }

        file.close();

        return true;
    }

    void insert(const Dataset<T_IN, T_OUT> &d) { data.insert(data.end(), d.data.begin(), d.data.end()); }
};

// using Dataset = std::vector<std::pair<std::bitset<72>, std::bitset<9>>>;

using DS = Dataset<std::vector<float>, std::vector<float>>;

std::pair<std::vector<float>, std::vector<float>> get_sample(maze<21, 31> m, int x, int y) {
    clean_maze<21, 31>(m);
    size_t start_score = pass_maze<21, 31>(m);
    std::vector<float> input;

    // сначала добавим стенки лабиринта
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 31; j++) {
            input.push_back(m[i][j] < MX ? 0 : 1);
        }
    }

    // теперь добавим путь
    size_t max_steps = 0;

    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 31; j++) {
            if (m[i][j] >= MX) {
                continue;
            }
            if (m[i][j] > max_steps) {
                max_steps = m[i][j];
            }
        }
    }

    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 31; j++) {
            if (m[i][j] >= MX) {
                input.push_back(0);
                continue;
            }
            input.push_back(static_cast<float>(m[i][j]) / max_steps);
        }
    }

    // теперь добавим квадрат 3x3 вокруг точки
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 31; j++) {
            if (i >= x - 1 && i <= x + 1 && j >= y - 1 && j <= y + 1) {
                input.push_back(1);
            } else {
                input.push_back(0);
            }
        }
    }
    clean_maze<21, 31>(m);

    std::vector<float> output;

    // переберём все возможные варианты в квадрате 3x3 вокруг точки

    assert(x >= 2 && x <= 18);
    assert(y >= 2 && y <= 28);

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

    for (int i = 0; i < 9; i++) {
        output.push_back(best_m[i]);
    }

    assert((pass_maze<21, 31>(m) == start_score));
    clean_maze<21, 31>(m);

    assert(input.size() == 21 * 31 * 3); // 1953 элемента на входе
    assert(output.size() == 9);          // 9 элементов на выходе

    return std::make_pair(input, output);
}

bool check_sampler(int num_samples) {
    maze<21, 31> m;
    prepare_maze<21, 31>(m);
    for (int i = 0; i < num_samples; i++) {
        int x = (r() % (18 - 2 + 1)) + 2;
        int y = (r() % (28 - 2 + 1)) + 2;
        auto [input, output] = get_sample(m, x, y);
        
    }
    return true;
}

DS get_dataset_from_search(size_t size, int num_updates) {
    DS dataset;
    maze<21, 31> m;
    prepare_maze<21, 31>(m);
    while (dataset.size() < size) {
        // для начала немного апдейтнем лабиринт
        MutationManager<21, 31> mm;

        size_t score = pass_maze<21, 31>(m);
        clean_maze<21, 31>(m);
        for (int i = 0; i < num_updates; i++) {
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

        auto [input, output] = get_sample(m, x, y);

        dataset.add(std::make_pair(input, output));
    }

    return dataset;
}

class Data_generator {
  public:
    enum Command { STOP, NONE };

    int num_workers, sample_size, num_updates;
    std::vector<std::optional<std::thread>> threads;
    std::vector<int> stats;
    std::thread manager_thread;
    std::atomic_flag manager_command_done; // only checks in manager thread, set in main thread
    std::atomic_flag manager_in_progress;  // only checks in main thread, set in manager thread
    Command manager_command;

    std::vector<DS> datasets;
    std::vector<std::atomic_flag> command_done;             // only checks in worker threads, set in manager thread
    std::vector<std::atomic_flag> computations_in_progress; // only checks in manager thread, set in worker threads
    Command command;
    DS dataset;

    std::mutex mtx;
    std::condition_variable cv;

    Data_generator(int num_workers, int sample_size, int num_updates)
        : num_workers(num_workers), sample_size(sample_size), num_updates(num_updates), threads(num_workers),
          stats(num_workers), datasets(num_workers), command_done(num_workers), computations_in_progress(num_workers),
          command(NONE) {
        for (size_t i = 0; i < num_workers; i++) {
            command_done[i].test_and_set();
            computations_in_progress[i].clear();
            stats[i] = 0;
        }

        manager_command_done.test_and_set();

        manager_thread = std::thread([this]() {
            std::unique_lock<std::mutex> lock(mtx);
            while (true) {
                cv.wait(lock);
                if (!manager_command_done.test_and_set()) {
                    if (manager_command == STOP) {
                        std::cout << "Stopping manager thread" << std::endl;

                        auto non_stopped = [&]() {
                            int non_stopped = 0;
                            for (size_t i = 0; i < this->num_workers; i++) {
                                if (!this->computations_in_progress[i].test_and_set()) {
                                    this->computations_in_progress[i].clear();
                                } else {
                                    non_stopped++;
                                }
                            }
                            return non_stopped;
                        };

                        command = STOP;

                        for (auto &flag : command_done) {
                            flag.clear();
                        }

                        while (auto non_stopped_count = non_stopped()) {
                            std::cout << "Waiting for " << non_stopped_count << " workers" << std::endl;
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        }

                        std::cout << "All workers stopped" << std::endl;

                        for (auto &t : this->threads) {
                            if (t)
                                t->join();
                        }
                        this->threads.clear();

                        for (auto &d : this->datasets) {
                            dataset.insert(d);
                        }

                        std::cout << "All datasets merged" << std::endl;

                        manager_in_progress.clear();
                        break;
                    }
                }

                for (size_t i = 0; i < this->num_workers; i++) {
                    if (!this->computations_in_progress[i].test_and_set()) {

                        if (this->threads[i]) {
                            this->threads[i]->join();
                        }

                        this->stats[i]++;

                        this->dataset.insert(this->datasets[i]);
                        this->datasets[i].clear();

                        this->threads[i] = std::thread([this, i]() { this->thread_worker(i); });
                    }
                }
            }
        });
    }

    void thread_worker(size_t index) {
        this->datasets[index] = get_dataset_from_search(sample_size, num_updates);
        this->computations_in_progress[index].clear();
        cv.notify_one();
    }

    void shuffle_dataset() { dataset.shuffle(); }

    // remove non unique elements
    void unique_dataset() { dataset.unique(); }

    bool write_dataset_to_file(const std::string &filename) {
        std::lock_guard<std::mutex> lock(mtx);

        std::cout << "Writing dataset to file " << filename << std::endl;
        std::cout << "Dataset size: " << dataset.size() << std::endl;

        // remove non unique elements
        // unique_dataset();

        // std::cout << "Unique dataset size: " << dataset.size() << std::endl;

        return dataset.write_to_file(filename);
    }

    bool read_dataset_from_file(const std::string &filename) {
        std::lock_guard<std::mutex> lock(mtx);

        std::cout << "Reading dataset from file " << filename << std::endl;
        std::cout << "Dataset size: " << dataset.size() << std::endl;

        DS d;
        if (!d.read_from_file(filename)) {
            return false;
        }

        dataset.insert(d);

        unique_dataset();

        std::cout << "New dataset size: " << dataset.size() << std::endl;

        return true;
    }

    size_t get_dataset_size() {
        std::lock_guard<std::mutex> lock(mtx);
        return dataset.size();
    }

    void notify() { cv.notify_all(); }

    void print_stats() {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t i = 0; i < num_workers; i++) {
            std::cout << "Worker " << i << " stats: " << stats[i] << std::endl;
        }
    }

    void write_to_file_as_csv(const std::string &filename) {
        std::lock_guard<std::mutex> lock(mtx);

        dataset.save_as_csv(filename);
    }

    ~Data_generator() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            manager_command = STOP;
            manager_command_done.clear();
            cv.notify_all();
        }
        manager_thread.join();

        write_dataset_to_file("autosave.bin");
    }
};

void data_generator_memu() {
    int num_workers = 20;
    int sample_size = 100;
    int num_updates = 10;

    std::cout << "Enter number of workers: ";
    std::cin >> num_workers;

    std::cout << "Enter sample size: ";
    std::cin >> sample_size;

    std::cout << "Enter number of updates: ";
    std::cin >> num_updates;

    std::cout << "Starting data generator with " << num_workers << " workers, sample size " << sample_size << " and "
              << num_updates << " updates" << std::endl;

    // std::cout << "Do not forget to manually notify to start computations" << std::endl;

    Data_generator dg(num_workers, sample_size, num_updates);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    dg.notify(); // start computations

    int command = 0;

    while (command != 1) {

        std::cout << "Commands:" << std::endl;
        std::cout << "1 - exit" << std::endl;
        std::cout << "2 - save dataset" << std::endl;
        std::cout << "3 - get dataset size" << std::endl;
        std::cout << "4 - manually notify" << std::endl;
        std::cout << "5 - print stats" << std::endl;
        std::cout << "6 - append from file" << std::endl;
        std::cout << "7 - write to csv" << std::endl;
        std::cout << "8 - show bin files" << std::endl;
        std::cout << "9 - read all bin files" << std::endl;
        std::cout << "Enter command: ";
        std::cin >> command;

        switch (command) {
        case 1:
            break;
        case 2: {
            std::cout << "Enter file name" << std::endl;
            std::string filename;
            std::cin >> filename;
            if (dg.write_dataset_to_file(filename)) {
                std::cout << "Dataset saved to " << filename << std::endl;
            } else {
                std::cout << "Error saving dataset" << std::endl;
            }
            break;
        }
        case 3:
            std::cout << "Dataset size: " << dg.get_dataset_size() << std::endl;
            break;
        case 4:
            dg.notify();
            break;
        case 5:
            dg.print_stats();
            break;
        case 6: {
            std::cout << "Enter file name" << std::endl;
            std::string filename;
            std::cin >> filename;
            if (dg.read_dataset_from_file(filename)) {
                std::cout << "Dataset read from " << filename << " ok" << std::endl;
            } else {
                std::cout << "Error reading dataset" << std::endl;
            }
            break;
        }
        case 7: {
            std::cout << "Enter file name" << std::endl;
            std::string filename;
            std::cin >> filename;
            dg.write_to_file_as_csv(filename);
            std::cout << "Dataset written to " << filename << std::endl;
            break;
        }
        case 8: {
            for (const auto &entry : std::filesystem::directory_iterator(".")) {
                if (entry.path().extension() == ".bin") {
                    Data_generator dd(1, 1000, 10);
                    dd.manager_command = Data_generator::STOP;
                    dd.read_dataset_from_file(entry.path().filename().string());
                    std::cout << entry.path().filename() << " size: " << dd.get_dataset_size() << std::endl;
                }
            }
            break;
        }
        case 9: {
            std::cout << "Size before: " << dg.get_dataset_size() << std::endl;
            for (const auto &entry : std::filesystem::directory_iterator(".")) {
                if (entry.path().extension() == ".bin") {
                    dg.read_dataset_from_file(entry.path().filename().string());
                    std::cout << entry.path().filename() << " size after: " << dg.get_dataset_size() << std::endl;
                }
            }
            break;
        }
        }
    }
}

int main() {

    data_generator_memu();

    return 0;
}
