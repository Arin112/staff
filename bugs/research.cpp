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
#include <list>
#include <exception>

using namespace std;

// Left to right, top to bottom
#define SIZEX 31
#define SIZEY 21
#define MX numeric_limits<size_t>::max()

using maze = size_t[SIZEX][SIZEY];

char symols_by_solidness[] = {' ', '.', ',', ':', ';', 'o', 'O', '0', '#'};

void print_with_solidness(maze m);

int r()
{
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<int> dis(0, 100000);
    return dis(gen);
}

inline void fix_borders(maze m)
{
    for (size_t i = 0; i < SIZEX; i++)
    {
        for (size_t j = 0; j < SIZEY; j++)
        {
            if (i == 0 || j == 0 || i == SIZEX - 1 || j == SIZEY - 1)
            {
                m[i][j] = MX;
            }
        }
    }
}

void clear_maze(maze m)
{
    fix_borders(m);
    for (size_t i = 1; i < SIZEX - 1; i++)
    {
        for (size_t j = 1; j < SIZEY - 1; j++)
        {
            m[i][j] = 0;
        }
    }
}

bool is_borders_valid(maze m)
{
    for (size_t i = 0; i < SIZEX; i++)
    {
        for (size_t j = 0; j < SIZEY; j++)
        {
            if (i == 0 || j == 0 || i == SIZEX - 1 || j == SIZEY - 1)
            {
                if (m[i][j] != MX)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void generate_inplace(maze m, bool fix_border = true)
{
    if (fix_border)
    {
        fix_borders(m);
    }
start_generate:
    for (size_t i = 1; i < SIZEX - 1; i++)
    {
        for (size_t j = 1; j < SIZEY - 1; j++)
        {
            m[i][j] = (r() % 2) ? MX : 0;
        }
    }
    m[1][1] = 0;
    m[SIZEX - 2][SIZEY - 2] = 0;

    if (m[1][2] == MX && m[2][1] == MX)
    {
        goto start_generate;
    }
    if (m[SIZEX - 3][SIZEY - 2] == MX && m[SIZEX - 2][SIZEY - 3] == MX)
    {
        goto start_generate;
    }
}

bool check_bfs(maze m)
{
    if (m[1][1] == MX || m[SIZEX - 2][SIZEY - 2] == MX)
    {
        return false;
    }
    queue<pair<size_t, size_t>> q;
    maze visited;
    clear_maze(visited);
    q.push({1, 1});
    visited[1][1] = 1;
    while (!q.empty())
    {
        auto [x, y] = q.front();
        q.pop();
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (abs(i) + abs(j) != 1)
                {
                    continue;
                }
                size_t nx = x + i;
                size_t ny = y + j;
                if (nx < SIZEX && ny < SIZEY && m[nx][ny] != MX && !visited[nx][ny])
                {
                    visited[nx][ny] = 1;
                    q.push({nx, ny});
                }
            }
        }
    }
    return visited[SIZEX - 2][SIZEY - 2];
}

void generate_inplace_valid(maze m)
{
    generate_inplace(m, true);
    while (!check_bfs(m))
    {
        generate_inplace(m, false);
    }
}

void dump_to_file(maze m, string filename, bool append = false)
{
    ofstream out(filename, append ? ios::app : ios::trunc);
    for (size_t i = 0; i < SIZEX; i++)
    {
        for (size_t j = 0; j < SIZEY; j++)
        {
            out << (m[i][j] == MX ? "@" : " ");
        }
        out << endl;
    }
    out.close();
}

void read_from_file(maze m, string filename)
{
    ifstream in(filename);
    for (size_t i = 0; i < SIZEX; i++)
    {
        string s;
        in >> s;
        for (size_t j = 0; j < SIZEY; j++)
        {
            m[i][j] = (s[j] == '@' ? MX : 0);
        }
    }
    in.close();
}

bool is_same_maze(maze m1, maze m2)
{
    for (size_t i = 0; i < SIZEX; i++)
    {
        for (size_t j = 0; j < SIZEY; j++)
        {
            if (m1[i][j] != m2[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

void copy_maze_to(maze m1, maze m2)
{
    for (size_t i = 0; i < SIZEX; i++)
    {
        for (size_t j = 0; j < SIZEY; j++)
        {
            m2[i][j] = m1[i][j];
        }
    }
}

size_t correct_algo(maze m)
{
    maze visited;
    clear_maze(visited);
    copy_maze_to(m, visited);
    int x = 1, y = 1;
    size_t steps = 0;

    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, 1, 0, -1};
    int cur_direction = 0;

    while (x != SIZEX - 2 || y != SIZEY - 2)
    {
        visited[x][y]++;

        size_t min_visits = MX;
        int next_x = x, next_y = y;
        int next_direction = -1;

        for (int i = 0; i < 4; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx >= 0 && nx < SIZEX && ny >= 0 && ny < SIZEY && visited[nx][ny] != MX && visited[nx][ny] <= min_visits)
            {
                if (visited[nx][ny] < min_visits || (visited[nx][ny] == min_visits && i == cur_direction))
                {
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
    // print_with_solidness(visited);
    return steps;
}


#define MIN(a, b) ((a) < (b) ? (a) : (b))
size_t algo_2(maze m)
{
    maze visited;
    clear_maze(visited);
    copy_maze_to(m, visited);
    int x = 1, y = 1;
    size_t steps = 0;

    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, 1, 0, -1};
    int cur_direction = 0;

    while (x != SIZEX - 2 || y != SIZEY - 2)
    {
        visited[x][y]++;

        size_t min_visits = MIN(MIN(visited[x+1][y], visited[x][y+1]), MIN(visited[x-1][y], visited[x][y-1]));
        
        if (min_visits == visited[x+dx[cur_direction]][y+dy[cur_direction]]){
            x = x + dx[cur_direction];
            y = y + dy[cur_direction];
        } else {
            int min_direction = min_visits == visited[x+1][y] ? 0 : min_visits == visited[x][y+1] ? 1 : min_visits == visited[x-1][y] ? 2 : 3;
            x = x + dx[min_direction];
            y = y + dy[min_direction];
            cur_direction = min_direction;
        }
        steps++;
    }
    // print_with_solidness(visited);
    return steps;
}

bool check_algorithms_equal(function<size_t(maze)> f1, function<size_t(maze)> f2, int n = 1000)
{
    for (int i = 0; i < n; i++)
    {
        maze m;
        generate_inplace_valid(m);
        size_t correct = f1(m);
        size_t steps = f2(m);
        if (steps != correct)
        {
            cout << "Test " << i + 1 << " failed" << endl;
            return false;
        }
        cout << "Test " << i + 1 << " passed" << endl;
    }
    cout << "All tests passed" << endl;
    return true;
}

void mutate_maze(maze m, int mutate_count = 2)
{
    for (int i = 0; i < mutate_count; i++)
    {
        size_t x = r() % (SIZEX - 2) + 1;
        size_t y = r() % (SIZEY - 2) + 1;
        m[x][y] = (m[x][y] == MX ? 0 : MX);
    }
    m[1][1] = 0;
    m[SIZEX - 2][SIZEY - 2] = 0;
}

void print_maze(maze m)
{
    for (size_t i = 0; i < SIZEX; i++)
    {
        for (size_t j = 0; j < SIZEY; j++)
        {
            cout << (m[i][j] == MX ? "@" : " ");
        }
        cout << endl;
    }
}

bool bfs_from(maze m, size_t x, size_t y){
    if (m[x][y] == MX){
        return false;
    }
    queue<pair<size_t, size_t>> q;
    maze visited;
    clear_maze(visited);
    q.push({x, y});
    visited[x][y] = 1;
    while (!q.empty())
    {
        auto [x, y] = q.front();
        q.pop();
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (abs(i) + abs(j) != 1)
                {
                    continue;
                }
                size_t nx = x + i;
                size_t ny = y + j;
                if (nx < SIZEX && ny < SIZEY && m[nx][ny] != MX && !visited[nx][ny])
                {
                    visited[nx][ny] = 1;
                    q.push({nx, ny});
                }
            }
        }
    }
    return visited[SIZEX - 2][SIZEY - 2];
}

size_t volume_of(maze m, size_t x, size_t y){
    if (m[x][y] == MX){
        return 0;
    }
    queue<pair<size_t, size_t>> q;
    maze visited;
    clear_maze(visited);
    q.push({x, y});
    visited[x][y] = 1;
    size_t volume = 1;
    while (!q.empty())
    {
        auto [x, y] = q.front();
        q.pop();
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (abs(i) + abs(j) != 1)
                {
                    continue;
                }
                size_t nx = x + i;
                size_t ny = y + j;
                if (nx < SIZEX && ny < SIZEY && m[nx][ny] != MX && !visited[nx][ny])
                {
                    visited[nx][ny] = 1;
                    q.push({nx, ny});
                    volume++;
                }
            }
        }
    }
    return volume;
}

// with ascii

void print_with_solidness(maze m){
    size_t max_val = 0;
    for(size_t i = 0; i < SIZEX; i++){
        for(size_t j = 0; j < SIZEY; j++){
            if (m[i][j] != MX){
                max_val = max(max_val, m[i][j]);
            }
        }
    }
    cout << "Max val: " << max_val << endl;
    for(size_t i = 0; i < SIZEX; i++){
        for(size_t j = 0; j < SIZEY; j++){
            if (m[i][j] == MX){
                cout << "@";
            } else {
                cout << symols_by_solidness[m[i][j] * (sizeof(symols_by_solidness)-1) / max_val];
            }
        }
        cout << endl;
    }
    cout << endl;
}

class ResearchDataBase{
public:
    virtual ~ResearchDataBase() = default;
    virtual string to_string() = 0;
    virtual void from_string(string s) = 0;
    virtual string serialize() = 0;
    virtual void deserialize(string s) = 0;
};

template<typename T>
class ResearcData : public ResearchDataBase{
public:
    T data;
    ResearcData(T data) : data(data){}
    string to_string() override{
        return data.to_string();
    }
    void from_string(string s) override{
        data.from_string(s);
    }
};

class ResearchBase{
    unique_ptr<ResearchDataBase> data;

public:
    virtual ~ResearchBase() = default;

    virtual bool iterate() = 0;
};


class ResearchManager{
    public:
    ResearchManager(){
        if (!filesystem::exists("research")){
            filesystem::create_directory("research");
        }
    }

    void run(){
    }
};




int main()
{

    maze m;
    generate_inplace_valid(m);
    clear_maze(m);

    size_t steps = 0, mx = 0;

    while (true)
    {
        maze m2;
        copy_maze_to(m, m2);
        mutate_maze(m2, rand() % 4 + 1);
        /*
        if (!is_borders_valid(m2))
        {
            cout << "Error in mutate_maze" << endl;
            return 1;
        }
        */
        if (check_bfs(m2))
        {
            size_t steps2 = algo_2(m2);
            /*
            size_t steps3 = algo_2(m2);
            if(steps2 != steps3){
                cout << "Error in algo_2" << endl;
                return 1;
            }
            */
            if (steps2 > steps*0.99-10)
            {
                copy_maze_to(m2, m);
                if (steps2 > mx)
                {
                    system("cls");
                    cout << "Steps: " << steps2 << endl;
                    print_maze(m);
                    mx = steps2;
                    
                }
                steps = steps2;
            }
        }
    }

    return 0;
}
