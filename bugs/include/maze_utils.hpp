#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace utils {

using crd = unsigned char;
struct point {
    crd x, y;
};

template <crd M, crd N>
using maze = size_t[M][N];

template <crd M, crd N>
using bset = std::bitset<(M - 2) * (N - 2) - 2>;

constexpr auto MX = std::numeric_limits<size_t>::max() - 5;

const char symols_by_solidness[] = {' ', '.', ',', ':', ';', 'o', 'O', '0', '#'};

int r();

template <crd M, crd N>
void show_maze(maze<M, N> m, bool solidness = false) {
    size_t max_val = 0;
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if (m[i][j] < MX) {
                max_val = std::max(max_val, m[i][j]);
            }
        }
    }
    std::cout << "Max val: " << max_val << std::endl;
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if (m[i][j] >= MX) {
                std::cout << "@";
            } else {
                if (solidness) {
                    std::cout
                        << symols_by_solidness[m[i][j] * (sizeof(symols_by_solidness) - 1) / (max_val ? max_val : 1)];
                } else {
                    std::cout << " ";
                }
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template <crd M, crd N>
std::string to_buglab_format(maze<M, N> m) {
    std::string res;
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            res += (m[i][j] >= MX ? "#" : ".");
        }
        res += "\n";
    }
    return res;
}

template <crd M, crd N>
std::string to_extended_format(maze<M, N> m) {
    std::string res;
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if(m[i][j] >= MX){
                res += " ## ";
                continue;
            }
            std::stringstream ss;
            ss << std::setw(2) << m[i][j];
            std::string s = ss.str();
            res += " " + s + " ";
        }
        res += "\n";
    }
    return res;
}

template <crd M, crd N>
void from_buglab_format(const std::string &s, maze<M, N> &m) {
    size_t k = 0;
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            m[i][j] = (s[k] == '#' ? MX : 0);
            k++;
        }
        k += 2;
    }
}

template <crd M, crd N>
inline void fix_borders(maze<M, N> m) {
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) {
                m[i][j] = MX;
            }
        }
    }
}

template <crd M, crd N>
void clear_maze(maze<M, N> m) {
    fix_borders<M, N>(m);
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            m[i][j] = 0;
        }
    }
}

template <crd M, crd N>
void clean_maze(maze<M, N> m) {
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            m[i][j] = m[i][j] < MX ? 0 : m[i][j];
        }
    }
}

template <crd M, crd N>
bool is_borders_valid(maze<M, N> m) {
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) {
                if (m[i][j] != MX) {
                    return false;
                }
            }
        }
    }
    return true;
}

template <crd M, crd N>
void prepare_maze(maze<M, N> m) {
    fix_borders<M, N>(m);
    clear_maze<M, N>(m);
}

template <crd M, crd N>
void randomize_maze(maze<M, N> m) {
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            m[i][j] = r() % 2 ? MX : 0;
        }
    }
    m[1][1] = 0;
    m[M - 2][N - 2] = 0;
}

template <crd M, crd N>
bool is_solvable(maze<M, N> m, crd y = 1, crd x = 1) {

    if (x < 1 || y < 1 || y >= M - 1 || x >= N - 1) {
        return false;
    }

    if (m[y][x] >= MX || m[M - 2][N - 2] >= MX) {
        return false;
    }
    std::queue<std::pair<crd, crd>> q;
    maze<M, N> visited;
    prepare_maze<M, N>(visited);
    q.push({y, x});
    visited[y][x] = 1;
    while (!q.empty()) {
        auto [y, x] = q.front();
        q.pop();
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (abs(i) + abs(j) != 1) {
                    continue;
                }
                crd nx = x + i;
                crd ny = y + j;
                if (m[ny][nx] < MX && !visited[ny][nx]) {
                    visited[ny][nx] = 1;
                    q.push({ny, nx});
                }
            }
        }
    }
    return visited[M - 2][N - 2];
}

template <crd M, crd N>
void generate_solvable_maze(maze<M, N> m) {
    prepare_maze<M, N>(m);
    do {
        randomize_maze<M, N>(m);
    } while (!is_solvable<M, N>(m));
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
template <crd M, crd N>
size_t pass_maze(maze<M, N> m) {
    int x = 1, y = 1;
    size_t steps = 0;

    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, 1, 0, -1};
    int cur_direction = 0;

    while (x != M - 2 || y != N - 2) {
        m[x][y]++;

        size_t min_visits = MIN(MIN(m[x + 1][y], m[x][y + 1]), MIN(m[x - 1][y], m[x][y - 1]));

        if (min_visits == m[x + dx[cur_direction]][y + dy[cur_direction]]) {
            x = x + dx[cur_direction];
            y = y + dy[cur_direction];
        } else {
            int min_direction = min_visits == m[x + 1][y]   ? 0
                                : min_visits == m[x][y + 1] ? 1
                                : min_visits == m[x - 1][y] ? 2
                                                                  : 3;
            x = x + dx[min_direction];
            y = y + dy[min_direction];
            cur_direction = min_direction;
        }
        steps++;
    }
    // print_with_solidness(visited);
    return steps;
}
#undef MIN

template <crd M, crd N>
size_t get_max(maze<M, N> m) {
    size_t max = 0;
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if (m[i][j] < MX) {
                max = std::max(max, m[i][j]);
            }
        }
    }
    return max;
}

template <crd M, crd N>
size_t get_min(maze<M, N> m) {
    size_t min = MX;
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            if (i == 1 && j == 1)
                continue;
            if (i == M - 2 && j == N - 2)
                continue;
            if (m[i][j] == 0)
                continue;
            if (m[i][j] < MX) {
                min = std::min(min, m[i][j]);
            }
        }
    }
    return min;
}

template <crd M, crd N>
size_t get_median(maze<M, N> m) {
    std::vector<size_t> v;
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            if (i == 1 && j == 1)
                continue;
            if (i == M - 2 && j == N - 2)
                continue;
            if (m[i][j] < MX) {
                v.push_back(m[i][j]);
            }
        }
    }
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

template <size_t C, crd M, crd N>
struct Mutation {
    point points[C];
    Mutation() { randomize(); }
    void randomize() {
        for (size_t i = 0; i < C; i++) {
            crd x = 1 + r() % (N - 2);
            crd y = 1 + r() % (M - 2);
            if (x == 1 && y == 1) {
                i--;
                continue;
            }
            if (x == N - 2 && y == M - 2) {
                i--;
                continue;
            }
            points[i] = {x, y};
        }
    }
    void apply(maze<M, N> m) {
        for (size_t i = 0; i < C; i++) {
            m[points[i].y][points[i].x] = m[points[i].y][points[i].x] >= MX ? 0 : MX;
        }
    }
};
/*
template<size_t N>
auto createTuple() { // example of creating tuple with N elements, each element is 0
    return std::apply([](auto... index) {
        return std::make_tuple((static_cast<void>(index), 0)...);
    }, std::make_index_sequence<N>{});
}

template <typename TupleT, typename Fn>
void for_each_tuple(TupleT&& tp, Fn&& fn) {
    std::apply
    (
        [&fn]<typename ...T>(T&& ...args)
        {
            (fn(std::forward<T>(args)), ...);
        }, std::forward<TupleT>(tp)
    );
}
*/

template <crd M, crd N>
bset<M, N> maze_to_bitset(maze<M, N> m) {
    bset<M, N> res;
    size_t k = 0;
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            if (i == 1 && j == 1)
                continue;
            if (i == M - 2 && j == N - 2)
                continue;
            res[k] = m[i][j] >= MX;
            k++;
        }
    }
    return res;
}

template <crd M, crd N>
void bitset_to_maze(bset<M, N> bs, maze<M, N> &m) {
    prepare_maze<M, N>(m);
    size_t k = 0;
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            if (i == 1 && j == 1)
                continue;
            if (i == M - 2 && j == N - 2)
                continue;
            m[i][j] = bs[k] ? MX : 0;
            k++;
        }
    }
}

template <crd M, crd N>
void increment_maze(maze<M, N> &m) {
    for (crd i = 1; i < M - 1; i++) {
        for (crd j = 1; j < N - 1; j++) {
            if (i == 1 && j == 1)
                continue;
            if (i == M - 2 && j == N - 2)
                continue;
            if (m[i][j] < MX) {
                m[i][j] = MX;
                return;
            }
            m[i][j] = 0;
        }
    }
}

template <crd M, crd N>
void copy_maze(maze<M, N> &source, maze<M, N> &dist) {
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            dist[i][j] = source[i][j];
        }
    }
}

template <crd M, crd N>
bool maze_equal(maze<M, N> &m1, maze<M, N> &m2) {
    for (crd i = 0; i < M; i++) {
        for (crd j = 0; j < N; j++) {
            if (m1[i][j] != m2[i][j]) {
                return false;
            }
        }
    }
    return true;
}

template <crd M, crd N>
class MutationManager {
    std::tuple<Mutation<1, M, N>, Mutation<2, M, N>, Mutation<3, M, N>, Mutation<4, M, N>, Mutation<5, M, N>> mutations;
    size_t last_mutation = 0;

  public:
    void randomize_mutation(size_t i) {
        switch (i) {
        case 0:
            std::get<0>(mutations).randomize();
            break;
        case 1:
            std::get<1>(mutations).randomize();
            break;
        case 2:
            std::get<2>(mutations).randomize();
            break;
        case 3:
            std::get<3>(mutations).randomize();
            break;
        case 4:
            std::get<4>(mutations).randomize();
            break;
        default:
            throw std::runtime_error("Invalid mutation index");
            break;
        }
    }
    void apply_mutation(maze<M, N> m, size_t i) {
        switch (i) {
        case 0:
            std::get<0>(mutations).apply(m);
            break;
        case 1:
            std::get<1>(mutations).apply(m);
            break;
        case 2:
            std::get<2>(mutations).apply(m);
            break;
        case 3:
            std::get<3>(mutations).apply(m);
            break;
        case 4:
            std::get<4>(mutations).apply(m);
            break;
        default:
            throw std::runtime_error("Invalid mutation index");
            break;
        }
    }

    void apply_random_mutation(maze<M, N> m) {
        size_t r = utils::r() % 5;
        last_mutation = r;
        randomize_mutation(r);
        apply_mutation(m, r);
    }
    void deny_last_mutation(maze<M, N> m) { apply_mutation(m, last_mutation); }
};

} // namespace utils
