#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <limits>
#include <random>
#include <algorithm>
#include <queue>
#include <windows.h>
#include <filesystem>
#include <functional>

using namespace std;

// Left to right, top to bottom
#define SIZEX 31
#define SIZEY 21
#define MX numeric_limits<size_t>::max()

using maze = size_t[SIZEX][SIZEY];

int r(){
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<int> dis(0, 100000);
    return dis(gen);
} 

inline void fix_borders(maze m){
    for(size_t i = 0; i < SIZEX; i++){
        for(size_t j = 0; j < SIZEY; j++){
            if (i == 0 || j == 0 || i == SIZEX - 1 || j == SIZEY - 1){
                m[i][j] = MX;
            }
        }
    }
}

void clear_maze(maze m){
    fix_borders(m);
    for(size_t i = 1; i < SIZEX - 1; i++){
        for(size_t j = 1; j < SIZEY - 1; j++){
            m[i][j] = 0;
        }
    }
}

void generate_inplace(maze m, bool fix_border = true){
    if (fix_border){
        fix_borders(m);
    }
    start_generate:
    for(size_t i = 1; i < SIZEX - 1; i++){
        for(size_t j = 1; j < SIZEY - 1; j++){
            m[i][j] = (r() % 2) ? MX : 0;
        }
    }
    m[1][1] = 0;
    m[SIZEX - 2][SIZEY - 2] = 0;

    if (m[1][2] == MX && m[2][1] == MX){
        goto start_generate;
    }
    if (m[SIZEX - 3][SIZEY - 2] == MX && m[SIZEX - 2][SIZEY - 3] == MX){
        goto start_generate;
    }
}

bool check_bfs(maze m){
    queue<pair<size_t, size_t>> q;
    maze visited;
    clear_maze(visited);
    q.push({1, 1});
    visited[1][1] = 1;
    while(!q.empty()){
        auto [x, y] = q.front();
        q.pop();
        for(int i = -1; i <= 1; i++){
            for(int j = -1; j <= 1; j++){
                if (abs(i) + abs(j) != 1){
                    continue;
                }
                size_t nx = x + i;
                size_t ny = y + j;
                if (nx < SIZEX && ny < SIZEY && m[nx][ny] != MX && !visited[nx][ny]){
                    visited[nx][ny] = 1;
                    q.push({nx, ny});
                }
            }
        }
    }
    return visited[SIZEX - 2][SIZEY - 2];
}

void generate_inplace_valid(maze m){
    generate_inplace(m, true);
    while(!check_bfs(m)){
        generate_inplace(m, false);
    }
}

void dump_to_file(maze m, string filename, bool append = false){
    ofstream out(filename, append ? ios::app : ios::trunc);
    for(size_t i = 0; i < SIZEX; i++){
        for(size_t j = 0; j < SIZEY; j++){
            out << (m[i][j] == MX ? "@" : " ");
        }
        out << endl;
    }
    out.close();
}

void read_from_file(maze m, string filename){
    ifstream in(filename);
    for(size_t i = 0; i < SIZEX; i++){
        string s;
        in >> s;
        for(size_t j = 0; j < SIZEY; j++){
            m[i][j] = (s[j] == '@' ? MX : 0);
        }
    }
    in.close();
}

bool is_same_maze(maze m1, maze m2){
    for(size_t i = 0; i < SIZEX; i++){
        for(size_t j = 0; j < SIZEY; j++){
            if (m1[i][j] != m2[i][j]){
                return false;
            }
        }
    }
    return true;
}

bool test_file_io(int n = 1000){
    for(int i = 0; i < n; i++){
        maze m1, m2;
        generate_inplace_valid(m1);
        dump_to_file(m1, "test.txt");
        read_from_file(m2, "test.txt");
        if (!is_same_maze(m1, m2)){
            return false;
        }
        cout << "Test " << i + 1 << " passed" << endl;
    }
    return true;
}

// get the number of steps using correct_1.exe < test.txt
size_t call_correct_1(maze m){
    ofstream out("test.txt");
    out << SIZEX << " " << SIZEY << endl;
    dump_to_file(m, "test.txt", true);
    out.close();
    system("correct_1.exe < test.txt > out.txt");
    ifstream in("out.txt");
    size_t steps;
    in >> steps;
    in.close();
    return steps;
}

void generate_store_tests(int n = 100){
    // Create a directory for the tests
    filesystem::create_directory("tests");
    for(int i = 0; i < n; i++){
        maze m;
        generate_inplace_valid(m);
        dump_to_file(m, "tests/test_" + to_string(i) + ".txt");
    }
    // create a file with answers
    ofstream out("tests/answers.txt");
    for(int i = 0; i < n; i++){
        maze m;
        read_from_file(m, "tests/test_" + to_string(i) + ".txt");
        out << call_correct_1(m) << endl;
    }
}

void check_program(){
    ifstream in("tests/answers.txt");
    size_t ans;
    int i = 0;
    while (in >> ans){
        maze m;
        read_from_file(m, "tests/test_" + to_string(i) + ".txt");
        size_t steps = call_correct_1(m);
        if (steps != ans){
            cout << "Test " << i + 1 << " failed" << endl;
            return;
        }
        cout << "Test " << i + 1 << " passed" << endl;
        i++;
    }
    cout << "All tests passed" << endl;
}

void copy_maze_to(maze m1, maze m2){
    for(size_t i = 0; i < SIZEX; i++){
        for(size_t j = 0; j < SIZEY; j++){
            m2[i][j] = m1[i][j];
        }
    }
}

size_t correct_algo(maze m){
    maze visited;
    clear_maze(visited);
    copy_maze_to(m, visited);
    int x = 1, y = 1;
    size_t steps = 0;
 
    int dx[] = { 1, 0, -1, 0 };
    int dy[] = { 0, 1, 0, -1 };
    int cur_direction = 0;
 
    while (x != SIZEX - 2 || y != SIZEY - 2) {
        visited[x][y]++;
 
        size_t min_visits = MX;
        int next_x = x, next_y = y;
        int next_direction = -1;
 
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
 
            if (nx >= 0 && nx < SIZEX && ny >= 0 && ny < SIZEY && visited[nx][ny] != MX && visited[nx][ny] <= min_visits) {
                if (visited[nx][ny] < min_visits || (visited[nx][ny] == min_visits && i == cur_direction)) {
                    min_visits = visited[nx][ny];
                    next_x = nx;
                    next_y = ny;
                    next_direction = i;
                }
            }
        }

        x = next_x;
        y = next_y;
        cur_direction = next_direction;
        steps++;
    }
 
    return steps;
}

void check_algorithm(function<size_t(maze)> f, int n = 1000){
    for(int i = 0; i < n; i++){
        maze m;
        generate_inplace_valid(m);
        size_t correct = call_correct_1(m);
        size_t steps = f(m);
        if (steps != correct){
            cout << "Test " << i + 1 << " failed" << endl;
            return;
        }
        cout << "Test " << i + 1 << " passed" << endl;
    }
    cout << "All tests passed" << endl;

}

bool check_algorithms_equal(function<size_t(maze)> f1, function<size_t(maze)> f2, int n = 1000){
    for(int i = 0; i < n; i++){
        maze m;
        generate_inplace_valid(m);
        size_t correct = f1(m);
        size_t steps = f2(m);
        if (steps != correct){
            cout << "Test " << i + 1 << " failed" << endl;
            return false;
        }
        cout << "Test " << i + 1 << " passed" << endl;
    }
    cout << "All tests passed" << endl;
    return true;
}



int main() {

    // test_file_io(1000);
    // generate_store_tests(100);
    // check_program();
    // check_algorithm(my_algorithm, 1000);
    check_algorithms_equal(correct_algo, correct_algo, 1000);
    // check_algorithms_equal(correct_algo, correct_algo_simd, 1000);
    return 0;
}
