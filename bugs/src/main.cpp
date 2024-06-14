#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>
#include <filesystem>
#include <bitset>
#include <fstream>

#include <maze_utils.hpp>
#include <researcher.hpp>

using namespace utils;

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
bool test_fast_bruteforce(size_t max_threads = 8){
    Researcher<M, N> r;
    bool result = true;
    for (size_t i=1; i < max_threads; i++){
        result &= r.check_threaded_find_best_bruteforce(i);
        if (result == false){
            std::cout << "Test failed for " << M << "x" << N << " with " << i << " threads" << std::endl;
            break;
        }else{
            // std::cout << "Test passed for " << M << "x" << N << " with " << i << " threads" << std::endl;
        }
    }
    return result;
}

template <size_t K> // 2**K is max combinations
void small_researchers(){
    auto researchers = createResearchers<128, K>();

    std::cout << "Researchers count: " << std::tuple_size<decltype(researchers)>::value << std::endl;

    std::vector<std::thread> threads; // workers

    for_each_tuple(researchers, [&](auto &i) {
        auto [m, n] = i.get_dimensions();
        auto combinations = i.get_combinations();
        // std::cout << "Maze " << m << "x" << n << " combinations: " << combinations << std::endl;
        threads.push_back(std::thread([&i]() { i.threaded_find_best_bruteforce(); }));
    });

    for (auto &t : threads) {
        t.join();
    }

    for_each_tuple(researchers, [](auto &i) {
        i.extended_show_maze();
    });
}

void strange_researcher(){
    Researcher<21, 31> r;

    r.show_maze();
}

int main() {

    // remove saves folder
    // std::filesystem::remove_all("./saves");

    small_researchers<36>();

    return 0;
}
