#include <iostream>
#include <vector>
#include <string>
#include <fstream>
 
using namespace std;
 
int main() {
    int n, m;
    cin >> n >> m;
    vector<string> maze(n);
    getline(cin, maze[0]);
    for (int i = 0; i < n; ++i) {
        getline(cin, maze[i]);
    }
    vector<vector<int>> visited(n, vector<int>(m, 0));
    int x = 1, y = 1;
    int steps = 0;
 
    int dx[] = { 1, 0, -1, 0 };
    int dy[] = { 0, 1, 0, -1 };
    int cur_direction = 0;
 
    while (x != n - 2 || y != m - 2) {
        visited[x][y]++;
 
        int min_visits = 1e9;
        int next_x = x, next_y = y;
        int next_direction = -1;
 
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
 
            if (nx >= 0 && nx < n && ny >= 0 && ny < m && maze[nx][ny] != '@' && visited[nx][ny] <= min_visits) {
                if (visited[nx][ny] < min_visits || (visited[nx][ny] == min_visits && i == cur_direction)) {
                    min_visits = visited[nx][ny];
                    next_x = nx;
                    next_y = ny;
                    next_direction = i;
                }
            }
        }
 
        if (next_x == x && next_y == y) {
            cout << -1;
            return 0;
        }
 
        x = next_x;
        y = next_y;
        cur_direction = next_direction;
        steps++;
        if (steps > 1e7) {
            cout << -1;
            return 0;
        }
    }
 
    cout << steps;
 
    return 0;
}