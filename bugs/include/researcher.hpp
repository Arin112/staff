#pragma once

#include "maze_utils.hpp"

#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <functional>

using namespace utils;

template <crd M, crd N>
class Researcher {
  public:
    maze<M, N> m;
    constexpr static crd m_ = M;
    constexpr static crd n_ = N;
    bool found_best_by_bruteforce;
    std::string uniq_id;

    Researcher() : found_best_by_bruteforce(false), uniq_id("") {
        prepare_maze<M, N>(m);
        from_file("./saves");
    }

    Researcher(std::string id, std::string path) : found_best_by_bruteforce(false), uniq_id(id) {
        prepare_maze<M, N>(m);
        from_file(path);
    }

    Researcher(const Researcher &r) : found_best_by_bruteforce(r.found_best_by_bruteforce), uniq_id("copy_"+r.uniq_id) {
        for (crd i = 0; i < M; i++)
            for (crd j = 0; j < N; j++)
                m[i][j] = r.m[i][j];
    }

    Researcher(Researcher &&r) : found_best_by_bruteforce(r.found_best_by_bruteforce), uniq_id(r.uniq_id) {
        for (crd i = 0; i < M; i++)
            for (crd j = 0; j < N; j++)
                m[i][j] = r.m[i][j];
    }

    bool operator==(const Researcher &r) const {
        for (crd i = 0; i < M; i++)
            for (crd j = 0; j < N; j++)
                if (m[i][j] != r.m[i][j])
                    return false;
        if (found_best_by_bruteforce != r.found_best_by_bruteforce)
            return false;
        return true;
    }

    bool checked_write_to_file(const std::string &path) {
        Researcher<M, N> r;
        to_file(path);
        r.from_file(path);
        return *this == r;
    }

    void to_file(const std::string &path) {

        // если нет такой папки, то создать
        std::filesystem::create_directory(path);

        std::string filename = path + "/" + "Research_" + uniq_id + "_" + std::to_string(M) + "x" + std::to_string(N) + ".txt";
        std::ofstream file(filename);
        file << "found_best_by_bruteforce: " << found_best_by_bruteforce << std::endl;
        file << "Maze:" << std::endl;
        file << to_buglab_format<M, N>(m);
        file.close();
    }

    void from_file(const std::string &path) {
        std::string filename = path + "/" + "Research_" + uniq_id + "_" + std::to_string(M) + "x" + std::to_string(N) + ".txt";
        
        std::ifstream file(filename);

        if (!file.is_open()) {
            // std::cout << "File not found: " << filename << std::endl;
            return;
        }

        std::string field_name;

        do{
            if(! (file >> field_name) ){
                break;
            }

            if (field_name == "found_best_by_bruteforce:") {
                file >> found_best_by_bruteforce;
            } else if (field_name == "Maze:") {
                for (crd i = 0; i < M; i++) {
                    std::string line;
                    file >> line;
                    // std::cout << "Found line: " << line << std::endl;
                    for (crd j = 0; j < N; j++) {
                        m[i][j] = line[j] == '#' ? MX : 0;
                    }
                }
            }
        } while (!file.eof());
        std::cout << "Loaded maze " + filename << std::endl;
    }

    void find_best_bruteforce() {

        if (found_best_by_bruteforce) {
            return;
        }

        size_t max_combinations = size_t(1) << (((M - 2) * (N - 2)) - 2);
        // std::cout << "Max combinations: " << max_combinations << std::endl;
        size_t best_score = 0;
        maze<M, N> best_maze;
        maze<M, N> current_maze;

        auto is_maze_empty = [](maze<M, N> &m) -> bool {
            for (crd i = 1; i < M - 1; i++)
                for (crd j = 1; j < N - 1; j++)
                    if (m[i][j] > 0)
                        return false;
            return true;
        };

        prepare_maze<M, N>(current_maze);
        prepare_maze<M, N>(best_maze);

        increment_maze<M, N>(current_maze);
        size_t iter = 0;
        while (!is_maze_empty(current_maze)) {
            iter++;

            if (iter % 10000000 == 0) {
                std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
                std::cout << "Progress: " << (double(iter) / max_combinations) * 100. << "%     ";
                // std::cout << to_buglab_format<M, N>(current_maze) << std::endl;
            }

            // std::cout << to_buglab_format<M, N>(current_maze) << std::endl;
            if (is_solvable<M, N>(current_maze) == false) {
                // std::cout << "Not solvable" << std::endl;
                increment_maze<M, N>(current_maze);
                continue;
            } else {
                // std::cout << "Solvable" << std::endl;
            }
            size_t current_score = pass_maze<M, N>(current_maze);
            clean_maze<M, N>(current_maze);
            // std::cout << "Score: " << current_score << std::endl;
            if (current_score > best_score) {
                best_score = current_score;
                copy_maze<M, N>(current_maze, best_maze);
                // std::cout << "New best score: " << best_score << std::endl;
                // std::cout << to_buglab_format<M, N>(best_maze) << std::endl;
            }
            increment_maze<M, N>(current_maze);
        }
        this->found_best_by_bruteforce = true;

        if (iter > 10000000) {
            std::cout << std::endl;
        }
        copy_maze<M, N>(best_maze, m);
    }

    // fast bruteforce
    void threaded_find_best_bruteforce(size_t max_threads=12, bool show_progress=false){
        if (found_best_by_bruteforce) {
            return;
        }

        size_t max_combinations = size_t(1) << (((M - 2) * (N - 2)) - 2);

        std::vector<std::thread> threads;
        std::vector<size_t> scores(max_threads, 0);
        std::vector<bset<M, N>> mazes(max_threads, bset<M, N>());

        auto is_maze_empty = [](maze<M, N> &m) -> bool {
            for (crd i = 1; i < M - 1; i++)
                for (crd j = 1; j < N - 1; j++)
                    if (m[i][j] > 0)
                        return false;
            return true;
        };

        auto worker = [&](size_t thread_id) {
            maze<M, N> current_maze;
            prepare_maze<M, N>(current_maze);
            increment_maze<M, N>(current_maze); // skip first maze cause it's empty
            for (int i=0;i < thread_id; i++){
                increment_maze<M, N>(current_maze); // shift to start position by thread_id
            }

            auto time_start = std::chrono::high_resolution_clock::now();

            size_t iter = 0;
            do{
                iter+=max_threads;

                if (show_progress && iter % 100000000 == 0) {
                    auto now = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - time_start).count();
                    auto progress = (double(iter) / max_combinations);
                    auto time_left = (duration / progress) - duration;
                    std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
                    std::cout << "Thread " << thread_id << " progress: " << (double(iter) / max_combinations) * 100. << "%; left " << time_left/3600. << " h         ";
                }
                if (iter > max_combinations) {
                    break;
                }

                if (is_solvable<M, N>(current_maze) == false) {
                    for (int i=0;i < max_threads; i++){
                        increment_maze<M, N>(current_maze);
                    }
                    continue;
                }

                size_t current_score = pass_maze<M, N>(current_maze);
                clean_maze<M, N>(current_maze);

                if (current_score > scores[thread_id]) {
                    scores[thread_id] = current_score;
                    mazes[thread_id] = maze_to_bitset<M, N>(current_maze);
                }
                
                for (int i=0;i < max_threads; i++){
                    increment_maze<M, N>(current_maze);
                }
            } while (true);
        };

        for (size_t i=0; i < max_threads; i++){
            threads.push_back(std::thread(worker, i));
        }

        for (auto &t : threads) {
            t.join();
        }

        //std::cout << std::endl;
        // std::cout << "Threads finished" << std::endl;

        size_t best_score = 0;
        bset<M, N> best_maze;
        for (size_t i=0; i < max_threads; i++){
            if (scores[i] > best_score){
                best_score = scores[i];
                best_maze = mazes[i];
            }
        }

        bitset_to_maze<M, N>(best_maze, m);
        this->found_best_by_bruteforce = true;

        // std::cout << "Best score: " << best_score << std::endl;
    }

    bool check_threaded_find_best_bruteforce(size_t max_threads){
        this->found_best_by_bruteforce = false;
        auto time_start = std::chrono::high_resolution_clock::now();
        threaded_find_best_bruteforce(max_threads);
        auto time_end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
        maze<M, N> m1;
        copy_maze<M, N>(m, m1);
        this->found_best_by_bruteforce = false;
        time_start = std::chrono::high_resolution_clock::now();
        find_best_bruteforce();
        time_end = std::chrono::high_resolution_clock::now();
        clean_maze<M, N>(m1);
        clean_maze<M, N>(m);
        
        std::cout << "On maze " << int(M) << "x" << int(N) << " with " << max_threads << " threads: " << std::endl;
        std::cout << "Threaded time: " << duration << " ms" << std::endl;
        std::cout << "Bruteforce time: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count() << " ms" << std::endl;

        // std::cout << "threaded_find_best_bruteforce:" << std::endl;
        // std::cout << to_buglab_format<M, N>(m1) << std::endl;
        // std::cout << "find_best_bruteforce:" << std::endl;
        // std::cout << to_buglab_format<M, N>(m) << std::endl;

        size_t score1 = pass_maze<M, N>(m1);
        size_t score2 = pass_maze<M, N>(m);
        // std::cout << "Score1: " << score1 << std::endl;
        // std::cout << "Score2: " << score2 << std::endl;
        return score1 == score2;
    }

    void show_maze() { std::cout << to_buglab_format<M, N>(m) << std::endl; }

    void extended_show_maze() {
        std::cout << "Maze " << int(M) << "x" << int(N) << " score: " << get_score() << std::endl;
        std::cout << to_extended_format<M, N>(m) << std::endl;
    }

    size_t get_score() {
        clean_maze<M, N>(m);
        return pass_maze<M, N>(m);
    }

    size_t get_combinations() {
        return size_t(1) << (((M - 2) * (N - 2)) - 2); // 10000000000 - больше уже нет смысла перебирать
    }

    std::pair<int, int> get_dimensions() { return std::make_pair(M, N); }

    
    void iterate_mutation(std::function<size_t(maze<M, N>)> f) {
        MutationManager<21, 31> mm;
    }

    ~Researcher() {
        clean_maze<M, N>(m);
        to_file("./saves");
    }
};